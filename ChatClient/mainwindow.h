#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <winsock2.h>
#include "recvthread.h"
#include "protocol.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void receiveBuffer(const char*);

private slots:
    void on_pushButton_clicked();
    void on_comboBox_currentIndexChanged(int index);
    void on_pushButton_2_clicked();

private:
    SOCKADDR_IN     servAddr;
    SOCKET          sock;
    RecvThread*     recv_thread;
    int             len;
    Ui::MainWindow  *ui;

    void processLoginReplyPkt(const char* pkt);
    void processPublicChatPkt(const char* pkt);
    void processPrivateChatPkt(const char* pkt);
    void appendText(QString from, QString msg, bool isPrivate = false, bool isMe = false);
    void ErrorHandling(char *message);
};

#endif // MAINWINDOW_H
