#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myftp.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    UdpSocket.bind(8081);
    connect(&UdpSocket,SIGNAL(readyRead()),SLOT(GetMessage()));

    client_list = QVector<myftp*>();
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
    UdpSocket.writeDatagram(data,QHostAddress::LocalHost, 8080);
}

void MainWindow::GetMessage()
{
    QByteArray data;
    data.resize(UdpSocket.pendingDatagramSize());

    // 接收数据的来源ip和端口
    QHostAddress ip;
    quint16 port;
    UdpSocket.readDatagram(data.data(), data.size(),&ip,&port);

    QString command;
    QDataStream in(&data,QIODevice::ReadOnly);
    in>>command;
    if(command == "new connect")
    {
        this->new_connect(ip, port);
    }
    else if(command == "get dir")
    {
        this->get_dir(ip,port);
    }
    else if(command == "download file")
    {
        int i;
        QString file_name;
        in>>file_name>>i;
        this->download_file(ip,port,file_name,i);
    }
    else if (command == "upload file data") {
        this->upload_file_data(ip,port,&in);
    }
    else if (command == "upload file end") {
        this->upload_file_end(ip,port,&in);
    }
    qDebug()<<command;
    this->InsertRecord(ip.toString()+":"+QString::number(port)+"  "+command);
}

// 按键，更改共享目录地址
void MainWindow::on_pushButton_clicked()
{
    QString dir_url = QFileDialog::getExistingDirectory();
    this->ui->lineEdit->setText(dir_url);
}

// 发送文件
void MainWindow::SendFile(QString file_name, int want_count)
{
    QString file_path = this->ui->lineEdit->text()+file_name;
    QFile file;
    file.setFileName(file_path);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"wrong" ;
        return;
    }

    int count=0;
    while(!file.atEnd()){
        QByteArray line;
        QDataStream out3(&line, QIODevice::WriteOnly);
        out3<<QString("download file data")<<file_name<<count<<file.read(4000);

        if(count>=want_count)
        {
            UdpSocket.writeDatagram(line,QHostAddress::LocalHost,8080);
            return;
        }
        count++;
    }
    QByteArray data2;
    QDataStream out2(&data2, QIODevice::WriteOnly);
    out2<<QString("download file end")<<file_name<<count;
    UdpSocket.writeDatagram(data2,QHostAddress::LocalHost,8080);
}

// 新连接
void MainWindow::new_connect(QHostAddress ip,int port)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    for(int i=0;i<this->client_list.size();i++)
    {
        // 查看该地址是否已经建立连接
        if(client_list.at(i)->Is_equal(ip,port))
        {
            // 如果已经建立连接
            // 发送错误信息
            out<<QString("error")<<QString("already connect");
            client_list.at(i)->SendMessage(data);
            return;
        }
    }
    // 当之前没有建立连接，则新加入
    myftp* new_one = new myftp(ip,port,this);
    client_list.append(new_one);
    // 发送连接成功
    out<<QString("connect success");
    new_one->SendMessage(data);

    // 向列表添加新用户
    qDebug()<<ui->tableWidget->rowCount()<<endl;
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
    qDebug()<<ui->tableWidget->rowCount()<<endl;
    qDebug()<<ip.toString()<<QString::number(port);
    ui->tableWidget->setItem(
                ui->tableWidget->rowCount()-1,0,
                new QTableWidgetItem(ip.toString()));
    ui->tableWidget->setItem(
                ui->tableWidget->rowCount()-1,1,
                new QTableWidgetItem(QString::number(port)));
    qDebug()<<ui->tableWidget->rowCount()<<endl;
}

// 获取目录信息
QString MainWindow::GetFilePath()
{
    return this->ui->lineEdit->text();
}

// 添加一条记录
void MainWindow::InsertRecord(QString record)
{
    this->ui->textEdit->append(record);
}

void MainWindow::get_dir(QHostAddress ip, int port)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    for(int i=0;i<this->client_list.size();i++)
    {
        // 查看该地址是否已经建立连接
        if(client_list.at(i)->Is_equal(ip,port))
        {
            // 如果已经建立连接，直接响应请求
            client_list.at(i)->SendDir();
            return;
        }
    }
    // 如果没有建立连接
    // 发送错误信息
    out<<QString("error")<<QString("haven't connect");
    UdpSocket.writeDatagram(data,ip,port);
}

void MainWindow::download_file(QHostAddress ip, int port, QString file_name, int want_count)
{

    for(int i=0;i<this->client_list.size();i++)
    {
        // 查看该地址是否已经建立连接
        if(client_list.at(i)->Is_equal(ip,port))
        {
            // 如果已经建立连接，直接响应请求
            client_list.at(i)->SendFile(file_name,want_count);
            return;
        }
    }

    // 如果没有建立连接
    // 发送错误信息
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out<<QString("error")<<QString("already connect");
    UdpSocket.writeDatagram(data,ip,port);
}

void MainWindow::upload_file_data(QHostAddress ip, int port, QDataStream *in)
{

    for(int i=0;i<this->client_list.size();i++)
    {
        // 查看该地址是否已经建立连接
        if(client_list.at(i)->Is_equal(ip,port))
        {
            // 如果已经建立连接，直接响应请求
            client_list.at(i)->ReceiveFile(in);
            return;
        }
    }

    // 如果没有建立连接
    // 发送错误信息
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out<<QString("error")<<QString("already connect");
    UdpSocket.writeDatagram(data,ip,port);
}

void MainWindow::upload_file_end(QHostAddress ip, int port, QDataStream *in)
{
    for(int i=0;i<this->client_list.size();i++)
    {
        // 查看该地址是否已经建立连接
        if(client_list.at(i)->Is_equal(ip,port))
        {
            // 如果已经建立连接，直接响应请求
            client_list.at(i)->ReceiveEnd(in);
            return;
        }
    }

    // 如果没有建立连接
    // 发送错误信息
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out<<QString("error")<<QString("already connect");
    UdpSocket.writeDatagram(data,ip,port);
}
