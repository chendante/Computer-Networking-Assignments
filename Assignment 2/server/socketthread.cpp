#include"socketthread.h"
#include<QString>

socketThread::socketThread(QWidget *parent, qintptr p,MainWindow *the_window):QThread(parent)
{
    this->ptr = p;
    this->main_window = the_window;
}

void socketThread::run()
{
    mytcpsocket * socket = new mytcpsocket(this->parent(),this->ptr);
    socket->waitForBytesWritten();
}
