#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QTimer>
#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include <QTcpServer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void NewConnect();
    void SendMessage();
    void GetMessage();
    void SendDir();
    
private:
    Ui::MainWindow *ui;
    QUdpSocket m_UdpSocket,g_UdpSocket;
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
};

#endif // MAINWINDOW_H
