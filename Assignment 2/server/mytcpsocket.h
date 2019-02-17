#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include<QTcpSocket>
#include<QWidget>
#include<QString>
#include<QByteArray>
#include"mainwindow.h"

class mytcpsocket: public QObject
{
    Q_OBJECT
public:
    mytcpsocket(QTcpSocket*,MainWindow*);
    QTcpSocket* m_tcp;
    MainWindow *pp;
    int status;     //标识当前同客户端交互的状态
    QByteArray m_data;  //保存邮件正文内容
    QByteArray frombase64(const QByteArray&);


signals:
    void sendString(QString);

public slots:
    void getMessage();  //获取客户端发送信息
    void deal(QString); //判断客户端发送内容种类，并作相应处理
    void dealMessage(QByteArray);   //用于处理客户端发送的命令行
    void dealContent();     //用于处理邮件正文内容
    void savefile(QByteArray);

};

#endif // MYTCPSOCKET_H
