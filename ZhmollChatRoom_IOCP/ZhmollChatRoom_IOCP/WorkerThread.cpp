#include <iostream>
#include <process.h>

#include "WorkerThread.h"
#include "Utils.h"
#include "Protocol.h"
#include "Process.h"

void ReadHandler() {

}

void WriteHandler() {
	// �������
	//puts("message sent!");
	//delete(ioInfo);
}

unsigned int __stdcall WorkerThread(LPVOID pComPort) // �����߳�
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
		// handleInfo ioInfo�����������õ���
		hClntSock = handleInfo->hClntSock;

		if (ioInfo->rwMode == MODE_READ) {
			// �����
			ioInfo->recvSize += recvSize;
			ioInfo->wsaBuf.buf = ioInfo->buffer + ioInfo->recvSize;
			ioInfo->wsaBuf.len = BUF_SIZE - ioInfo->recvSize;
			// �����ر��׽���
			if (ioInfo->recvSize <= 0) {
				puts("A client has left.\n");
				closesocket(hClntSock);
				delete handleInfo;
				delete ioInfo;
				continue;
			}
			// �����ò���length
			if (ioInfo->recvSize < 8) {
				WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, NULL, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
				continue;
			}
			// ��������ı��ĳ����Ѿ��õ�������ԭ��û���õ���
			if (ioInfo->expectedSize == 0) {
				ioInfo->expectedSize = ((Header*)(ioInfo->buffer))->length;
			}
			// �յ���С�ڰ����ȡ�������Ҫ����һ�Σ�ճ��
			if (ioInfo->recvSize < ioInfo->expectedSize) {
				WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, NULL, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
				continue;
			}
			// �յ��Ĵ��ڵ��ڰ����ȡ�������Ҫ�ְ�����processPkt����
			while (ioInfo->recvSize >= ioInfo->expectedSize) {
				char* completedPkt = new char[ioInfo->expectedSize];
				memcpy(completedPkt, ioInfo->buffer, ioInfo->expectedSize);
				processPkt(completedPkt, handleInfo);
				delete[] completedPkt;

				// ���·ְ�����������Ⱥ��յ�����
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
			// �������
			delete ioInfo;
		}
	}
	return 0;
}
