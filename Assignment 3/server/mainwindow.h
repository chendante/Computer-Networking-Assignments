#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QTimer>
#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include <QVector>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QString>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QFileDialog>

namespace Ui {
class MainWindow;
}

class myftp;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void SendFile(QString file_name, int want_count);
    QString GetFilePath();
    void InsertRecord(QString);

private slots:
    void SendMessage();
    void GetMessage();

    void new_connect(QHostAddress ip,int port);
    void get_dir(QHostAddress ip,int port);
    void download_file(QHostAddress ip,int port,QString file_name,int want_count);
    void upload_file_data(QHostAddress ip,int port,QDataStream* in);
    void upload_file_end(QHostAddress ip,int port,QDataStream* in);
    
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QUdpSocket UdpSocket;
    QVector<myftp*> client_list;
};

#endif // MAINWINDOW_H
