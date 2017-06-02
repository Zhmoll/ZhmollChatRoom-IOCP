#pragma once
#include "Protocol.h"
#include "Utils.h"

#define PROCESS_ERROR   -1
#define PROCESS_SUCCEED 1

int processPkt(const char* pkt, HANDLE_DATA* handleInfo);
int processLoginPkt(const char* pkt, HANDLE_DATA* handleInfo);
int processLogoutPkt(const char* pkt, HANDLE_DATA* handleInfo);

int processPublicChatPkt(const char* pkt);
int processPrivateChatPkt(const char* pkt);

void broadcastPkt(const char* pkt, int length);