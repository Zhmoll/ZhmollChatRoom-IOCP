#ifndef PROTOCOL_H
#define PROTOCOL_H
// 协议

#define PKT_TYPE_UNKNOWN        -1
#define PKT_TYPE_LOGIN          1
#define PKT_TYPE_LOGOUT         2
#define PKT_TYPE_LOG_REPLY      3
#define PKT_TYPE_MSG_PUBLIC     11
#define PKT_TYPE_MSG_PRIVATE    12

typedef struct Header {
    int     type;
    int     length;
}Header;

typedef struct LoginPkt {
    Header  header;
    char    username[20];
    char    password[20];
}LoginPkt;

typedef struct LogoutPkt {
    Header  header;
    char    username[20];
}LogoutPkt;

#define LOGIN_FAIL	  0
#define LOGIN_SUCCEED 1

typedef struct LoginRelpyPkt {
    Header header;
    int	   retCode;
}LoginRelpyPkt;

typedef struct PublicChatMsgHeader {
    Header header;
    char   from[20];
}PublicChatMsgHeader;

typedef struct PrivateChatMsgHeader {
    Header header;
    char   from[20];
    char   to[20];
}PrivateChatMsgHeader;

#define SEND_ERROR          -1
#define SEND_USER_NOT_FOUND 0
#define SEND_SUCCEED        1

typedef struct SendMsgRelpyPkt {
    Header header;
    int	   retCode;
}SendMsgRelpyPkt;

#endif
