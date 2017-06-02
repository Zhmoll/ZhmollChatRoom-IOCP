#include <iostream>
#include <winsock2.h>
#include "Process.h"
#include "Session.h"
#include "Utils.h"

// 控制入口，这里接收到的包都是已经分装好的
int processPkt(const char* pkt, HANDLE_DATA* handleInfo) {
	int type = ((Header*)pkt)->type;

	switch (type) {
	case PKT_TYPE_LOGIN:
		return processLoginPkt(pkt, handleInfo);
	case PKT_TYPE_LOGOUT:
		return processLogoutPkt(pkt, handleInfo);
	case PKT_TYPE_MSG_PUBLIC:
		return processPublicChatPkt(pkt);
	case PKT_TYPE_MSG_PRIVATE:
		return processPrivateChatPkt(pkt);
	default://UNKNOWN_PKT
		return PROCESS_ERROR;
	}
}

// 处理登录包（完成）
int processLoginPkt(const char* pkt, HANDLE_DATA* handleInfo) {
	User user;
	LoginPkt* loginPkt = (LoginPkt*)pkt;

	strcpy(user.username, loginPkt->username);
	strcpy(user.password, loginPkt->password);

	int checkUserResult = checkUser(&user);
	LoginRelpyPkt replyPkt;

	switch (checkUserResult) {
	case USER_FOUND: {
		// 构造登录回复包
		replyPkt.header.type = PKT_TYPE_LOG_REPLY;
		replyPkt.header.length = sizeof(LoginRelpyPkt);
		replyPkt.retCode = LOGIN_SUCCEED;
		SendOut(handleInfo, (const char*)&replyPkt, sizeof(LoginRelpyPkt));

		// 添加会话
		addSession(loginPkt->username, handleInfo);

		// 构造消息头
		PublicChatMsgHeader PublicChatMsgHeader;
		PublicChatMsgHeader.header.type = PKT_TYPE_MSG_PUBLIC;
		strcpy(PublicChatMsgHeader.from, "system");

		// 构造消息体
		char msg[512] = { 0 };
		sprintf(msg, "%s entered Zhmoll TCP Chat Room.", loginPkt->username);

		// 拼装消息头和体
		int length = sizeof(PublicChatMsgHeader) + strlen(msg) + 1;
		PublicChatMsgHeader.header.length = length;
		char *buffer = new char[length];
		memcpy(buffer, &PublicChatMsgHeader, sizeof(PublicChatMsgHeader));
		memcpy(buffer + sizeof(PublicChatMsgHeader), msg, strlen(msg) + 1);

		// 广播
		broadcastPkt((const char*)buffer, length);
		delete[] buffer;
		break;
	}
	case USER_NOT_FOUND:
		replyPkt.header.type = PKT_TYPE_LOG_REPLY;
		replyPkt.retCode = LOGIN_FAIL;
		SendOut(handleInfo, (const char*)&replyPkt, sizeof(replyPkt));
		break;
	case USER_PASSWORD_ERROR:
		replyPkt.header.type = PKT_TYPE_LOG_REPLY;
		replyPkt.retCode = LOGIN_FAIL;
		SendOut(handleInfo, (const char*)&replyPkt, sizeof(replyPkt));
		break;
	}
	return 0;
}

// 处理注销包（完成）
int processLogoutPkt(const char* pkt, HANDLE_DATA* handleInfo) {
	const LogoutPkt* logoutPkt = (LogoutPkt*)pkt;
	removeSession(logoutPkt->username);

	PublicChatMsgHeader PublicChatMsgHeader;
	PublicChatMsgHeader.header.type = PKT_TYPE_MSG_PUBLIC;
	strcpy(PublicChatMsgHeader.from, "system");

	char msg[512] = { 0 };
	sprintf(msg, "%s Left Zhmoll TCP Chat Room.", logoutPkt->username);

	int length = sizeof(PublicChatMsgHeader) + strlen(msg) + 1;
	PublicChatMsgHeader.header.length = length;
	char *buffer = new char[length];
	memcpy(buffer, &PublicChatMsgHeader, sizeof(PublicChatMsgHeader));
	memcpy(buffer + sizeof(PublicChatMsgHeader), &msg, strlen(msg) + 1);

	broadcastPkt((const char*)buffer, length);
	delete[] buffer;
	return 0;
}

// 广播（完成）
void broadcastPkt(const char* pkt, int length) {
	Session* g_session = NULL;
	int g_session_count = getAllSession(&g_session);
	for (int i = 0; i < g_session_count; i++) {
		SendOut(g_session[i].handleInfo, (const char*)pkt, length);
	}
}

// 处理公聊包（完成）
int processPublicChatPkt(const char* pkt) {
	const PublicChatMsgHeader* header = (PublicChatMsgHeader*)pkt;
	HANDLE_DATA* handleInfo;

	if (SESSION_NOT_FOUND == findSession(header->from, &handleInfo) && strcmp(header->from, "system") != 0) {
		// 连发送者都找不到，不正常的包，就不理了
		return PROCESS_ERROR;
	}

	broadcastPkt(pkt, header->header.length);
	return PROCESS_SUCCEED;
}

// 处理私聊包（完成）
int processPrivateChatPkt(const char* pkt) {
	const PrivateChatMsgHeader* header = (PrivateChatMsgHeader*)pkt;
	HANDLE_DATA* handleInfo;

	if (SESSION_NOT_FOUND == findSession(header->to, &handleInfo)) {
		// 连发送者都找不到，不正常的包，就不理了
		return PROCESS_ERROR;
	}
	else {
		// 找到了要发送的用户
		SendOut(handleInfo, (const char*)pkt, header->header.length);
	}
	return PROCESS_SUCCEED;
}
