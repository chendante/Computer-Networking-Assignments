#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>
#include <QWidget>

class myserver:public QTcpServer
{
    Q_OBJECT

public:
    myserver(QWidget *parent);
    QWidget *pa;
//protected:
//    virtual void incomingConnection(qintptr socketDescriptor);
};

#endif // MYSERVER_H
