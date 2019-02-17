#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include<QThread>
#include<QTcpSocket>
#include<QWidget>
#include"mytcpsocket.h"
#include"mainwindow.h"

class socketThread :public QThread
{
private:
    qintptr ptr;
    mytcpsocket * socket;
public:
    socketThread(QWidget* parent,qintptr p);
    MainWindow *main_window;

protected:
    virtual void run();
};

#endif // SOCKETTHREAD_H
