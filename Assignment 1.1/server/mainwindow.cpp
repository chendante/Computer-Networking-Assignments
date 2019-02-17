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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    g_UdpSocket.bind(8081);
    connect(&g_UdpSocket,SIGNAL(readyRead()),SLOT(GetMessage()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SendMessage()
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QString test("使用UDP服务");
    out<<1<<QDateTime::currentDateTime()<<test;
    m_UdpSocket.writeDatagram(data,QHostAddress::LocalHost, 8080);
}

void MainWindow::SendDir()
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QDir dir("./");
    dir.setFilter(QDir::Files|QDir::Hidden|QDir::NoSymLinks);
    dir.setSorting(QDir::Time);
    QFileInfoList d_list = dir.entryInfoList();
    qDebug()<<d_list.size();
    //将d_list 中内容整理为JSON格式
    QJsonArray json_array;
    for (int i = 0; i < d_list.size(); i++)
    {
        QFileInfo fileInfo = d_list.at(i);
        QJsonObject file_json;
        file_json.insert("name",fileInfo.fileName());
        file_json.insert("time",fileInfo.created().toString("yyyy-MM-dd hh:mm:ss"));
        file_json.insert("size",QString::number(fileInfo.size()));
        file_json.insert("path",fileInfo.absoluteFilePath());
        json_array.append(QJsonValue(file_json));
    }
    QJsonDocument json_doc;
    json_doc.setArray(json_array);
    out<<2<<json_doc.toJson(QJsonDocument::Compact);
    m_UdpSocket.writeDatagram(data,QHostAddress::LocalHost, 8080);
}

void MainWindow::GetMessage()
{
    QByteArray data;
    while (g_UdpSocket.hasPendingDatagrams()) {
        data.resize(g_UdpSocket.pendingDatagramSize());
        g_UdpSocket.readDatagram(data.data(), data.size());
    }
    int Type;
    QString test;
    QDataStream in(&data,QIODevice::ReadOnly);
    in>>Type>>test;
    if(Type == 1)
    {
        this->SendMessage();
    }
    else if(Type == 2)
    {
        this->SendDir();
    }
    qDebug()<<test;
}
