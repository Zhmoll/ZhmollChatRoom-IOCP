#include <iostream>
#include <process.h>
#include <winsock2.h>

#include "Utils.h"
#include "WorkerThread.h"
#include "Session.h"

#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main(int argc, char *argv[]) {
	// ��������
	if (argc != 2) {
		printf("ʹ�÷���: %s <port>\n", argv[0]);
		exit(1);
	}

	// ���ݿ⣨α������
	if (USERDB_OPEN_FAIL == loadUserDB("users.txt")) {
		ErrorHandling("�û����ݿ��ʧ�ܣ�\n");
	}

	// WSAװ��
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() ִ�д���!");
	}

	// ������ɶ˿ھ��
	HANDLE hComPort;
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// ��ȡ��ǰϵͳ��Ϣ
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	// ������cpu������ͬ���߳�
	for (int i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, WorkerThread, (LPVOID)hComPort, 0, NULL);

	SOCKET hServSock;		// ��������׽���
	SOCKADDR_IN servAdr;	// ����˷����ַ

	// ���������ص�io���첽�����׽���
	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hServSock == INVALID_SOCKET) {
		ErrorHandling("TCP socket ��������");
	}

	// ��ʼ����������׽���
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	// ��
	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) {
		ErrorHandling("bind() ִ�д���");
	}

	// ����
	if (listen(hServSock, 5) == SOCKET_ERROR) {
		ErrorHandling("listen() ִ�д���");
	}

	// ���տͻ��˵�����
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
			ErrorHandling("accept() ִ�д���");
		}

		// �����Ϣ��ʼ��
		handleInfo = new HANDLE_DATA();
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		// ����socket�����֮ǰ��������ɶ˿ھ��
		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		// overlapped�ͻ����ʼ��
		ioInfo = new IO_DATA();
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = MODE_READ;

		// ע���������
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, (LPDWORD)&recvBytes, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
	}

	return 0;
}