#include <iostream>
#include <process.h>

#include "WorkerThread.h"
#include "Utils.h"
#include "Protocol.h"
#include "Process.h"

void ReadHandler() {

}

void WriteHandler() {
	// 发送完成
	//puts("message sent!");
	//delete(ioInfo);
}

unsigned int __stdcall WorkerThread(LPVOID pComPort) // 工作线程
{
	HANDLE hComPort = (HANDLE)pComPort;


	HANDLE_DATA* handleInfo;
	IO_DATA* ioInfo;
	DWORD flags = 0;

	SOCKET hClntSock;
	DWORD recvSize;
	while (true)
	{
		GetQueuedCompletionStatus(hComPort, &recvSize, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		// handleInfo ioInfo可以在这里拿到了
		hClntSock = handleInfo->hClntSock;

		if (ioInfo->rwMode == MODE_READ) {
			// 先组包
			ioInfo->recvSize += recvSize;
			ioInfo->wsaBuf.buf = ioInfo->buffer + ioInfo->recvSize;
			ioInfo->wsaBuf.len = BUF_SIZE - ioInfo->recvSize;
			// 错误或关闭套接字
			if (ioInfo->recvSize <= 0) {
				puts("A client has left.\n");
				closesocket(hClntSock);
				delete handleInfo;
				delete ioInfo;
				continue;
			}
			// 避免拿不到length
			if (ioInfo->recvSize < 8) {
				WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, NULL, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
				continue;
			}
			// 如果期望的报文长度已经拿到，并且原来没有拿到过
			if (ioInfo->expectedSize == 0) {
				ioInfo->expectedSize = ((Header*)(ioInfo->buffer))->length;
			}
			// 收到的小于包长度————要再收一次，粘包
			if (ioInfo->recvSize < ioInfo->expectedSize) {
				WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, NULL, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
				continue;
			}
			// 收到的大于等于包长度————要分包，给processPkt处理
			while (ioInfo->recvSize >= ioInfo->expectedSize) {
				char* completedPkt = new char[ioInfo->expectedSize];
				memcpy(completedPkt, ioInfo->buffer, ioInfo->expectedSize);
				processPkt(completedPkt, handleInfo);
				delete[] completedPkt;

				// 更新分包后的期望长度和收到长度
				ioInfo->recvSize = ioInfo->recvSize - ioInfo->expectedSize;
				memmove(ioInfo->buffer, ioInfo->buffer + ioInfo->expectedSize, ioInfo->recvSize);
				ioInfo->wsaBuf.buf = ioInfo->buffer + ioInfo->recvSize;
				ioInfo->wsaBuf.len = BUF_SIZE - ioInfo->recvSize;
				if (ioInfo->recvSize < 8) {
					ioInfo->expectedSize = 0;
					break;
				}
				else {
					ioInfo->expectedSize = ((Header *)(ioInfo->buffer))->length;
				}
			}

			WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, NULL, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
		}
		else
		{
			// 发送完成
			delete ioInfo;
		}
	}
	return 0;
}
