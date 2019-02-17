#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QTreeWidgetItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tcpSocket = new QTcpSocket(this);
    this->connectServer();
    connect(tcpSocket,QTcpSocket::readyRead,this,MainWindow::GetMessage);

    QStringList headerList;
    headerList.append(tr("文件名"));
    headerList.append(tr("创建时间"));
    headerList.append(tr("文件大小"));
    headerList.append(tr("文件路径"));
    ui->treeWidget->setHeaderLabels(headerList);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectServer()
{
    tcpSocket->abort();
    tcpSocket->connectToHost(QHostAddress::LocalHost,8081);
}

void MainWindow::GetMessage()
{
    QByteArray data = tcpSocket->readAll();
    qDebug()<<data;
    QDateTime time;
    QString test;
    int Type;
    QDataStream in(&data,QIODevice::ReadOnly);
    in>>Type;
    if(Type == Ttime)
    {
        in>>time>>test;
        ui->lineEdit_2->setText(test);
        ui->lineEdit->setText(time.time().toString());
        ui->lineEdit_3->setText(time.date().toString());
    }
    else if (Type == Tdir)
    {
        QJsonDocument readDoc;
        QByteArray json_data;
        json_data.resize(data.size()-sizeof(int));
        in>>json_data;
        qDebug()<<json_data;
        readDoc= QJsonDocument::fromJson(json_data);

        qDebug()<<readDoc.toJson();
        QJsonArray array_json = readDoc.array();
        for(int i = 0; i < array_json.size(); i++)
        {
            QJsonObject file_json(array_json.at(i).toObject());
            QTreeWidgetItem *newItem = new QTreeWidgetItem();
            newItem->setText(0,file_json.value("name").toString());
            newItem->setText(1,file_json.value("time").toString());
            newItem->setText(2,file_json.value("size").toString());
            newItem->setText(3,file_json.value("path").toString());
            ui->treeWidget->addTopLevelItem(newItem);
        }
    }
}

void MainWindow::SendMessage(int type)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QString test("有内鬼");
    qDebug()<<type;
    out<<type<<test;
//    s_UdpSocket.writeDatagram(data,QHostAddress::LocalHost, 8081);
    tcpSocket->write(data);
}

void MainWindow::on_pushButton_clicked()
{
    this->connectServer();
    //获取时间
    this->SendMessage(Ttime);
}

void MainWindow::on_pushButton_2_clicked()
{
    this->connectServer();
    this->SendMessage(Tdir);
}
