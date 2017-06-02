#include "recvthread.h"
#include "mainwindow.h"

RecvThread::RecvThread(SOCKET socket, QObject *parent):QThread(parent)
{
    sock = socket;
}

RecvThread::~RecvThread()
{

}

void RecvThread::run()
{
    SOCKET socket = sock;
    char buff[1500];
    int recvSize = 0;
    int expectedSize = 0;

    while(true){    
        recvSize += recv(socket, buff + recvSize, sizeof(buff) - recvSize, 0);

        // 错误或关闭套接字
        if (recvSize <= 0)
            break;

        // 避免拿不到length
        if (recvSize <8)
            continue;

        // 获取期望长度
        expectedSize = (expectedSize == 0 ? ((Header *)buff)->length : expectedSize);

        // 收到的小于包长度————要再收一次，粘包
        if (recvSize < expectedSize)
            continue;

        // 收到的大于等于包长度————要分包，给processPkt处理
        while (recvSize >= expectedSize) {
            char* completedPkt = new char[expectedSize];
            memcpy(completedPkt, buff, expectedSize);
            emit sendBuffer(completedPkt);
            delete[] completedPkt;

            // 更新分包后的期望长度和收到长度
            recvSize = recvSize - expectedSize;
            if (recvSize < 8) {
                memmove(buff, buff + expectedSize, recvSize);
                expectedSize = 0;
                break;
            }
            else{
                memmove(buff, buff + expectedSize, recvSize);
                expectedSize = ((Header *)buff)->length;
            }
        }
    }
}
