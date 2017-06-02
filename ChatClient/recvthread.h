#include <QThread>
#include <winsock2.h>
#include "protocol.h"

#ifndef RECVTHREAD_H
#define RECVTHREAD_H

class RecvThread:public QThread
{

    Q_OBJECT

public:
    RecvThread(SOCKET socket, QObject *parent = NULL);
    ~RecvThread();
    void run();

private:
    SOCKET sock;
    int len;

signals:
    void sendBuffer(const char*);
};

#endif // RECVTHREAD_H
