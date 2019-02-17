#ifndef MYFTP_H
#define MYFTP_H

#include "mainwindow.h"

class myftp: public QObject
{
    Q_OBJECT
public:
    myftp(QHostAddress,int,MainWindow*);
    MainWindow* pp;
    // 判断是否时当前客户
    bool Is_equal(QHostAddress,int);

    void SendMessage(QByteArray data);
    // 发送目录
    void SendDir();
    // 发送文件
    void SendFile(QString file_name, int want_count);
    // 接收文件
    void ReceiveFile(QDataStream* in);
    // 接收文件完毕
    void ReceiveEnd(QDataStream* in);
private:
    // 客户端端口信息
    QHostAddress client_host;
    int client_port;
    QUdpSocket UdpSocket;

    // 接收的文件数据
    QByteArray receiver_file_data;
    // 接收文件名
    QString receiver_file_name;
    // 接收文件片数目
    int receiver_count;
};
#endif // MYFTP_H
