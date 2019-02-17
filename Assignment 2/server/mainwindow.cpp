#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QString>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QTcpSocket>
#include <QThread>
#include "mytcpsocket.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tcpServer = new QTcpServer(this);
    //令tcpserver监听任何ip连接到本地25端口的命令
    if(!tcpServer->listen(QHostAddress::Any,25))
    {
        qDebug()<<tcpServer->errorString();
        close();
    }
    connect(tcpServer,&QTcpServer::newConnection,this,&MainWindow::NewConnect);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::GetMessage(QString str)
{
    QString now = this->ui->textEdit->toPlainText();
    now.append(str);
    this->ui->textEdit->setText(now);
}

void MainWindow::GetContent(QString str)
{
    this->ui->textBrowser->setHtml(str);
}

void MainWindow::NewConnect()
{
    this->GetMessage("*** 收到连接请求\r\n");
    mytcpsocket* tcp = new mytcpsocket(tcpServer->nextPendingConnection(),this);
}
