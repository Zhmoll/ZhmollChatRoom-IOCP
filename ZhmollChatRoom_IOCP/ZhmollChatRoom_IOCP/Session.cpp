#include <iostream>
#include <winsock2.h>
#include "Session.h"

User    g_users[MAX_USER_NUM];
int     g_users_count = 0;

Session g_online_sessions[MAX_USER_NUM];
int     g_online_sessions_count = 0;

// �����û����ݿ⣨��ɣ�
int loadUserDB(const char* fileName) {
	FILE* fp = fopen(fileName, "r");
	if (NULL == fp) {
		return USERDB_OPEN_FAIL;
	}

	while (!feof(fp)) {
		fscanf(fp, "%s%s", g_users[g_users_count].username, g_users[g_users_count].password);
		g_users_count++;
	}

	return 0;
}

// �����û���¼״̬����ɣ�
int checkUser(const User* user) {
	for (int i = 0; i < g_users_count; i++) {
		if (strcmp(g_users[i].username, user->username) == 0) {
			if (strcmp(g_users[i].password, user->password) == 0) {
				return USER_FOUND;
			}
			else {
				return USER_PASSWORD_ERROR;
			}
		}
	}
	return USER_NOT_FOUND;
}

// ����session��ָ�루��ɣ�
int getAllSession(Session** g_session) {
	*g_session = g_online_sessions;
	return g_online_sessions_count;
}

// ��ӻỰ״̬����ɣ�
int addSession(const char* username, HANDLE_DATA* handleInfo) {
	// todo: ��¼�û��ﵽ���޵Ĵ���
	strcpy(g_online_sessions[g_online_sessions_count].username, username);
	g_online_sessions[g_online_sessions_count].handleInfo = handleInfo;
	g_online_sessions_count++;
	return 0;
}

// �Ƴ��Ự״̬����ɣ�
int removeSession(const char* username) {
	for (int i = 0; i < g_online_sessions_count; i++) {
		if (strcmp(g_online_sessions[i].username, username) == 0) {
			for (int j = i; j < g_online_sessions_count - 1; j++) {
				g_online_sessions[j] = g_online_sessions[j + 1];
			}
			g_online_sessions_count--;
			return SESSION_REMOVE_SUCCEED;
		}
	}
	return SESSION_NOT_FOUND;
}

// Ѱ�һỰ����ɣ�
int findSession(const char* username, HANDLE_DATA** handleInfo) {
	SOCKET sock;
	for (int i = 0; i < g_online_sessions_count; i++) {
		if (strcmp(g_online_sessions[i].username, username) == 0) {
			*handleInfo = g_online_sessions[i].handleInfo;
			return SESSION_FOUND;
		}
	}
	return SESSION_NOT_FOUND;
}