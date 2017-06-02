#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        ErrorHandling("socket() error!");

    ui->setupUi(this);
    ui->textEdit->setReadOnly(true);
    ui->plainTextEdit_6->setVisible(false);
    ui->pushButton_2->setEnabled(false);
}

MainWindow::~MainWindow()
{
    if(ui->pushButton->text()=="注销"){
        QString username = ui->plainTextEdit_3->toPlainText();
        QByteArray usernameLatin = username.toLatin1();
        const char *cusername = usernameLatin.data();
        LogoutPkt logout;
        logout.header.type = PKT_TYPE_LOGOUT;
        strcpy(logout.username,cusername);
        send(sock, (const char*)(&logout), sizeof(logout), 0);
    }

    if(recv_thread!=NULL){
        recv_thread->terminate();
        closesocket(sock);
    }

    WSACleanup();
    delete ui;
}

// 登录按钮
void MainWindow::on_pushButton_clicked()
{
    QString host = ui->plainTextEdit->toPlainText();
    QString port = ui->plainTextEdit_2->toPlainText();
    QString username = ui->plainTextEdit_3->toPlainText();
    QString password = ui->plainTextEdit_4->toPlainText();
    QByteArray hostLatin = host.toLatin1();
    QByteArray portLatin = port.toLatin1();
    QByteArray usernameLatin = username.toLatin1();
    QByteArray passwordLatin = password.toLatin1();
    const char *chost = hostLatin.data();
    const char *cport = portLatin.data();
    const char *cusername = usernameLatin.data();
    const char *cpassword = passwordLatin.data();

    if(ui->pushButton->text()=="登录"){
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = inet_addr(chost);
        servAddr.sin_port = htons(atoi(cport));
        len = sizeof(servAddr);

        recv_thread = new RecvThread(sock,this);
        connect(recv_thread, SIGNAL(sendBuffer(const char*)), this, SLOT(receiveBuffer(const char*)), Qt::BlockingQueuedConnection);
        ::connect(sock,(SOCKADDR*)&servAddr,sizeof(servAddr));
        recv_thread->start();

        LoginPkt login;
        login.header.type = PKT_TYPE_LOGIN;
        login.header.length = sizeof(login);
        strcpy(login.username,cusername);
        strcpy(login.password,cpassword);
        send(sock, (const char*)(&login), sizeof(login), 0);

        ui->pushButton->setText("登录中");
        appendText("system", "登录中 ……");
    }else if(ui->pushButton->text()=="注销"){
        LogoutPkt logout;
        logout.header.type = PKT_TYPE_LOGOUT;
        logout.header.length = sizeof(logout);
        strcpy(logout.username,cusername);
        send(sock, (const char*)(&logout), sizeof(logout), 0);
        appendText("system","注销成功！");
        ui->pushButton->setText("登录");
        ui->pushButton_2->setEnabled(false);
        recv_thread->terminate();
    }
}

// 切换公聊私聊界面显示
void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if(index==0){
        ui->plainTextEdit_6->setVisible(false);
        ui->plainTextEdit_5->setGeometry(120,330,341,31);
    }else if(index==1){
        ui->plainTextEdit_6->setVisible(true);
        ui->plainTextEdit_5->setGeometry(230,330,231,31);
    }
}


void MainWindow::receiveBuffer(const char* pkt){
    int type = ((Header*)pkt)->type;
    switch (type) {
    case PKT_TYPE_LOG_REPLY:
        processLoginReplyPkt((const char*)pkt);
        break;

    case PKT_TYPE_MSG_PUBLIC:
        processPublicChatPkt((const char*)pkt);
        break;

    case PKT_TYPE_MSG_PRIVATE:
        processPrivateChatPkt((const char*)pkt);
        break;

    default:
        qDebug("错包 %d",type);
    }
}

void MainWindow::processLoginReplyPkt(const char* pkt){
    LoginRelpyPkt* reply = (LoginRelpyPkt*)pkt;
    QString str;
    if(reply->retCode == LOGIN_FAIL){
        str= "用户名或密码错误！";
    }
    else if(reply->retCode==LOGIN_SUCCEED){
        str= "登录成功！";
        ui->plainTextEdit->setReadOnly(true);
        ui->plainTextEdit_2->setReadOnly(true);
        ui->plainTextEdit_3->setReadOnly(true);
        ui->plainTextEdit_4->setReadOnly(true);
        ui->pushButton->setText("注销");
        ui->pushButton_2->setEnabled(true);
    }

    appendText("system",str);
}

void MainWindow::processPublicChatPkt(const char* pkt){
    PublicChatMsgHeader* header = (PublicChatMsgHeader*)pkt;
    QString msg((char *)(header+1));
    appendText(header->from, msg);
}

void MainWindow::processPrivateChatPkt(const char* pkt){
    PrivateChatMsgHeader* header = (PrivateChatMsgHeader*)pkt;
    QString msg((char *)(header+1));
    appendText(header->from, msg, true);
}

void MainWindow::appendText(QString from, QString msg, bool isPrivate, bool isMe){
    QString str;

    if (from == "system"){
        str = "<p><font color=\"red\"><b>[系统]："+msg+"</b></font></p>";
    }
    else if (!isPrivate){
        str="<p>["+from+"]："+msg+"</p>";
    }
    else if (!isMe){
        str="<p><font color=\"green\">["+from+"]对你说："+msg+"</font></p>";
    }
    else{
        str="<p><font color=\"green\">我对["+from+"]说："+msg+"</font></p>";
    }

    ui->textEdit->append(str);
}

void MainWindow::ErrorHandling(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}

void MainWindow::on_pushButton_2_clicked()
{
    QString from = ui->plainTextEdit_3->toPlainText();
    QByteArray fromLatin = from.toLatin1();
    const char *cfrom = fromLatin.data();

    QString to = ui->plainTextEdit_6->toPlainText();
    QByteArray toLatin = to.toLatin1();
    const char *cto = toLatin.data();

    QString content = ui->plainTextEdit_5->toPlainText();
    QByteArray contentLatin = content.toUtf8();
    const char *ccontent = contentLatin.data();

    if(ui->comboBox->currentIndex()==0){// 公聊
        PublicChatMsgHeader header;
        header.header.type = PKT_TYPE_MSG_PUBLIC;
        strcpy(header.from,cfrom);

        int length = sizeof(header)+strlen(ccontent)+1;
        header.header.length = length;
        char *buffer = new char[length];
        memcpy(buffer,&header,sizeof(header));
        memcpy(buffer+sizeof(header),ccontent,strlen(ccontent)+1);
        send(sock,(const char*)buffer,length,0);
        delete[] buffer;
    }else if(ui->comboBox->currentIndex()==1){// 私聊
        PrivateChatMsgHeader header;
        header.header.type = PKT_TYPE_MSG_PRIVATE;
        strcpy(header.from,cfrom);
        strcpy(header.to,cto);

        int length = sizeof(header)+strlen(ccontent)+1;
        header.header.length = length;
        char *buffer = new char[length];
        memcpy(buffer,&header,sizeof(header));
        memcpy(buffer+sizeof(header),ccontent,strlen(ccontent)+1);
        send(sock,(const char*)buffer,length,0);
        delete[] buffer;
        appendText(cto,ccontent,true,true);
    }
}
