#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonArray>
#include <QJsonObject>
#include <QTcpSocket>
#include <QHostAddress>

#define Ttime 1
#define Tdir  2

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
    void GetMessage();
    void SendMessage(int);
    void connectServer();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
};

#endif // MAINWINDOW_H
