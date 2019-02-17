#include"myserver.h"
#include"socketthread.h"
myserver::myserver(QWidget *parent):QTcpServer(parent){}

//void myserver::incomingConnection(qintptr socketDescriptor)
//{
//    socketThread * thread = new socketThread(this->pa, socketDescriptor);
//    thread->start();
//}
