#include <iostream>
#include <WinSock2.h>
#include "Utils.h"

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void SendOut(HANDLE_DATA* handleInfo, const char* data, const int length) {
	SOCKET hClntSock = handleInfo->hClntSock;
	IO_DATA* ioInfo = new IO_DATA();
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));

	memcpy(ioInfo->buffer, data, length);
	ioInfo->wsaBuf.buf = ioInfo->buffer;
	ioInfo->wsaBuf.len = length;
	ioInfo->rwMode = MODE_WRITE;

	WSASend(hClntSock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
}