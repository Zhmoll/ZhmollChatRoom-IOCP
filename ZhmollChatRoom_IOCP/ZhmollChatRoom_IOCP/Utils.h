#pragma once
#include <WinSock2.h>
#define BUF_SIZE 1500

#define MODE_READ 1
#define MODE_WRITE 2

typedef struct { // socket info  
	// ע���ʱ������ռ䡢���ݡ�ʹ��
	SOCKET hClntSock;
	SOCKADDR_IN clntAdr;
} HANDLE_DATA;

typedef struct { // buffer info
	// ��io�õ��Ļ����Overlapped�ṹ���װ��һ��
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode;    // MODE_READ or MODE_WRITE
	int expectedSize;
	int recvSize;
} IO_DATA;

void SendOut(HANDLE_DATA* handleInfo, const char* data, const int length);
void ErrorHandling(char *message);