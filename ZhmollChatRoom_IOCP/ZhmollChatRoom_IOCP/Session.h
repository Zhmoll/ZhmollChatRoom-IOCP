#pragma once
#include "Protocol.h"
#include "Utils.h"

#define MAX_USER_NUM 10000

#define SESSION_FOUND 0
#define SESSION_NOT_FOUND -1
#define SESSION_REMOVE_SUCCEED 1

#define USER_PASSWORD_ERROR	-2
#define USER_NOT_FOUND		-1
#define USER_FOUND			1

#define USERDB_OPEN_FAIL	-1

typedef struct User {
	char username[20];
	char password[20];
}User;

typedef struct Session {
	char username[20];
	HANDLE_DATA* handleInfo;
}Session;

int loadUserDB(const char* fileName);
int checkUser(const User* user);

int addSession(const char* userName, HANDLE_DATA* handleInfo);
int findSession(const char* username, HANDLE_DATA** handleInfo);
int removeSession(const char* userName);
int getAllSession(Session** g_session);
