#include <iostream>
#include <process.h>
#include <winsock2.h>

#include "Utils.h"
#include "WorkerThread.h"
#include "Session.h"

#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main(int argc, char *argv[]) {
	// 参数控制
	if (argc != 2) {
		printf("使用方法: %s <port>\n", argv[0]);
		exit(1);
	}

	// 数据库（伪）控制
	if (USERDB_OPEN_FAIL == loadUserDB("users.txt")) {
		ErrorHandling("用户数据库打开失败！\n");
	}

	// WSA装载
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() 执行错误!");
	}

	// 创建完成端口句柄
	HANDLE hComPort;
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// 获取当前系统信息
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	// 创建与cpu个数相同的线程
	for (int i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, WorkerThread, (LPVOID)hComPort, 0, NULL);

	SOCKET hServSock;		// 服务端用套接字
	SOCKADDR_IN servAdr;	// 服务端服务地址

	// 创建用于重叠io（异步）的套接字
	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hServSock == INVALID_SOCKET) {
		ErrorHandling("TCP socket 创建错误！");
	}

	// 初始化服务端用套接字
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	// 绑定
	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) {
		ErrorHandling("bind() 执行错误！");
	}

	// 监听
	if (listen(hServSock, 5) == SOCKET_ERROR) {
		ErrorHandling("listen() 执行错误！");
	}

	// 接收客户端的连接
	IO_DATA *ioInfo;
	HANDLE_DATA *handleInfo;
	int recvBytes;
	int flags = 0;
	while (true)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAdr;
		int addrLen = sizeof(clntAdr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);
		if (hClntSock == INVALID_SOCKET) {
			ErrorHandling("accept() 执行错误！");
		}

		// 句柄信息初始化
		handleInfo = new HANDLE_DATA();
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		// 连接socket句柄和之前创建的完成端口句柄
		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		// overlapped和缓冲初始化
		ioInfo = new IO_DATA();
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = MODE_READ;

		// 注册接收数据
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, (LPDWORD)&recvBytes, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
	}

	return 0;
}