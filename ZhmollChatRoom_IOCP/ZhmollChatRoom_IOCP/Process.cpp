#include <iostream>
#include <winsock2.h>
#include "Process.h"
#include "Session.h"
#include "Utils.h"

// ������ڣ�������յ��İ������Ѿ���װ�õ�
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

// �����¼������ɣ�
int processLoginPkt(const char* pkt, HANDLE_DATA* handleInfo) {
	User user;
	LoginPkt* loginPkt = (LoginPkt*)pkt;

	strcpy(user.username, loginPkt->username);
	strcpy(user.password, loginPkt->password);

	int checkUserResult = checkUser(&user);
	LoginRelpyPkt replyPkt;

	switch (checkUserResult) {
	case USER_FOUND: {
		// �����¼�ظ���
		replyPkt.header.type = PKT_TYPE_LOG_REPLY;
		replyPkt.header.length = sizeof(LoginRelpyPkt);
		replyPkt.retCode = LOGIN_SUCCEED;
		SendOut(handleInfo, (const char*)&replyPkt, sizeof(LoginRelpyPkt));

		// ��ӻỰ
		addSession(loginPkt->username, handleInfo);

		// ������Ϣͷ
		PublicChatMsgHeader PublicChatMsgHeader;
		PublicChatMsgHeader.header.type = PKT_TYPE_MSG_PUBLIC;
		strcpy(PublicChatMsgHeader.from, "system");

		// ������Ϣ��
		char msg[512] = { 0 };
		sprintf(msg, "%s entered Zhmoll TCP Chat Room.", loginPkt->username);

		// ƴװ��Ϣͷ����
		int length = sizeof(PublicChatMsgHeader) + strlen(msg) + 1;
		PublicChatMsgHeader.header.length = length;
		char *buffer = new char[length];
		memcpy(buffer, &PublicChatMsgHeader, sizeof(PublicChatMsgHeader));
		memcpy(buffer + sizeof(PublicChatMsgHeader), msg, strlen(msg) + 1);

		// �㲥
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

// ����ע��������ɣ�
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

// �㲥����ɣ�
void broadcastPkt(const char* pkt, int length) {
	Session* g_session = NULL;
	int g_session_count = getAllSession(&g_session);
	for (int i = 0; i < g_session_count; i++) {
		SendOut(g_session[i].handleInfo, (const char*)pkt, length);
	}
}

// �����İ�����ɣ�
int processPublicChatPkt(const char* pkt) {
	const PublicChatMsgHeader* header = (PublicChatMsgHeader*)pkt;
	HANDLE_DATA* handleInfo;

	if (SESSION_NOT_FOUND == findSession(header->from, &handleInfo) && strcmp(header->from, "system") != 0) {
		// �������߶��Ҳ������������İ����Ͳ�����
		return PROCESS_ERROR;
	}

	broadcastPkt(pkt, header->header.length);
	return PROCESS_SUCCEED;
}

// ����˽�İ�����ɣ�
int processPrivateChatPkt(const char* pkt) {
	const PrivateChatMsgHeader* header = (PrivateChatMsgHeader*)pkt;
	HANDLE_DATA* handleInfo;

	if (SESSION_NOT_FOUND == findSession(header->to, &handleInfo)) {
		// �������߶��Ҳ������������İ����Ͳ�����
		return PROCESS_ERROR;
	}
	else {
		// �ҵ���Ҫ���͵��û�
		SendOut(handleInfo, (const char*)pkt, header->header.length);
	}
	return PROCESS_SUCCEED;
}
