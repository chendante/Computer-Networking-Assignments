#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置目录显示窗口
    QStringList headerList;
    headerList.append(tr("文件名"));
    headerList.append(tr("创建时间"));
    headerList.append(tr("文件大小"));
    headerList.append(tr("文件路径"));
    ui->tableWidget->setColumnCount(4); //设置列数
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(150);
    ui->tableWidget->setHorizontalHeaderLabels(headerList);


    //默认绑定8080端口
    int port_num = 8080;
    //当该端口绑定不了时，切换其它端口
    while(!UdpSocket.bind(port_num))
    {
        port_num++;
    }
    connect(&UdpSocket,SIGNAL(readyRead()),SLOT(GetMessage()));

    //初始化ip地址和端口号
    this->remote_host.setAddress(ui->lineEdit->text());
    this->remote_port = ui->lineEdit_2->text().toInt();
    // -1代表当前没有文件正在上传或者下载
    this->download_count = -1;
    this->upload_count = -1;

    // 初始化两个定时器
    this->download_timer = new QTimer(this);
    this->upload_timer = new QTimer(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::GetMessage()
{
    QByteArray data;

    data.resize(UdpSocket.pendingDatagramSize());
    UdpSocket.readDatagram(data.data(), data.size());

    QString command;
    QDataStream in(&data,QIODevice::ReadOnly);
    in>>command;

    //根据命令不同，选用不同函数进行处理
    if(command == "connect success")
    {
        QMessageBox::information(this,tr("提示"),tr("连接建立成功"));
    }
    else if (command == "send dir")
    {
        this->Get_dir(&in);
    }
    else if(command == "download file data")
    {
        this->Get_data(&in);
    }
    else if(command == "download file end")
    {
        this->Get_download_end(&in);
    }
    else if(command == "upload file receive")
    {
        qDebug()<<command<<endl;
        this->Get_receive(&in);
    }
    else if(command == "upload file end")
    {
        this->Get_upload_end(&in);
    }
    else if(command == "error")
    {
        QString error_message;
        in>>error_message;
        this->Insert_record(command+" "+error_message);
        return;
    }
    this->Insert_record(command);
}

// 向服务器发送指令
void MainWindow::SendMessage(QString command)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);

    qDebug()<<command;
    out<<command;
    UdpSocket.writeDatagram(data,remote_host,remote_port);
}

void MainWindow::on_pushButton_2_clicked()
{
    this->SendMessage("get dir");
}

// 列表某行被双击，询问是否下载
void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QString file_name = ui->tableWidget->item(row, column)->text();
    switch(QMessageBox::question(this,tr("询问"),QString("是否下载 ")+file_name,
                QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok))
    {
    case QMessageBox::Ok:
        if(this->download_count == -1){
            this->download_file_data.clear();
            this->download_file_name = file_name;
            this->download_count = 0;
            Download_file();
        }
        else {
            QMessageBox::critical(this, tr("错误"),tr("有文件正在下载"));
        }
        break;
    case QMessageBox::Cancel:
        qDebug()<<"fou ";
        break;
    default:
        break;
    }
}

//发送下载文件请求
void MainWindow::Download_file()
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out<<QString("download file")<<download_file_name<<this->download_count;
    UdpSocket.writeDatagram(data,remote_host, remote_port);

    // 开始计时
    download_timer->stop();
    connect(download_timer,SIGNAL(timeout()),this,SLOT(Download_file()));
    download_timer->start(10000);

    Insert_record("download "+download_file_name+" count:"+download_count);
}

// 按下建立连接按钮
void MainWindow::on_pushButton_clicked()
{
    if(!this->remote_host.setAddress(ui->lineEdit->text()))
    {
        QMessageBox::critical(this, tr("错误"),tr("服务器地址填写错误"));
        return;
    }
    this->remote_port = ui->lineEdit_2->text().toInt();
    this->SendMessage("new connect");
}

// 用于获取数据的方法
void MainWindow::Get_data(QDataStream* in)
{
    QString file_name;
    *in>>file_name;
    if(file_name == this->download_file_name)
    {
        int count;
        *in>>count;
        // 如果获取的文件片是当前想要的文件片则处理
        if(count == download_count)
        {
            download_count++;
            QByteArray datagram;
            *in>>datagram;
            this->download_file_data.append(datagram.data());
        }
    }
    // 继续请求数据
    this->Download_file();
}

// 用于获取目录的方法
void MainWindow::Get_dir(QDataStream* in)
{
    QJsonDocument readDoc;
    QByteArray json_data;
    *in>>json_data;
    readDoc= QJsonDocument::fromJson(json_data);
    QJsonArray array_json = readDoc.array();
    ui->tableWidget->setRowCount(array_json.size());
    for(int i = 0; i < array_json.size(); i++)
    {
        QJsonObject file_json(array_json.at(i).toObject());
        this->ui->tableWidget->setItem(i,0,new QTableWidgetItem(file_json.value("name").toString()));
        this->ui->tableWidget->setItem(i,1,new QTableWidgetItem(file_json.value("time").toString()));
        this->ui->tableWidget->setItem(i,2,new QTableWidgetItem(file_json.value("size").toString()));
        this->ui->tableWidget->setItem(i,3,new QTableWidgetItem(file_json.value("path").toString()));
    }
}

// 用于处理接收到下载结束请求
void MainWindow::Get_download_end(QDataStream* in)
{
    qDebug()<<"get end"<<endl;
    QString file_name;
    *in>>file_name;
    if(file_name == this->download_file_name)
    {
        int count;
        *in>>count;
        if(count == download_count)
        {
            // 下载结束保存文件
            QFile file(this->download_file_name);
            if(!file.open(QIODevice::WriteOnly)) return;
            file.write(this->download_file_data.data(),download_file_data.size());
            file.close();

            // 将相关设置初始化
            this->download_count = -1;
            this->download_timer->stop();
            this->download_file_data.clear();
        }
    }
}

// 上传文件按钮
void MainWindow::on_pushButton_3_clicked()
{
    // 获取想要上传的文件名
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("上传文件"),
                                                    "/",
                                                    tr("任何文件(*.*)"
                                                        ";;文本文件(*.txt)"
                                                        ";;C++文件(*.cpp)"));
    if (fileName.length() != 0) {
        if(this->upload_count == -1)
        {
            this->upload_file_path = fileName;
            this->upload_count = 0;
            this->Upload_file();
        }
        else {
            QMessageBox::critical(this, tr("错误"),tr("当前正有文件上传错误"));
        }
        qDebug()<<fileName<<endl;
    }
}

// 添加新记录
void MainWindow::Insert_record(QString record)
{
    ui->textEdit->append(record);
}

// 上传文件
void MainWindow::Upload_file()
{
    QString file_name;
    int k = this->upload_file_path.lastIndexOf('/');
    file_name = this->upload_file_path.mid(k+1);
    qDebug()<<file_name;
    QFile file;
    file.setFileName(this->upload_file_path);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"wrong" ;
        return;
    }

    int count=0;
    while(!file.atEnd()){
        QByteArray line;
        QDataStream out(&line, QIODevice::WriteOnly);
        out<<QString("upload file data")<<file_name<<count<<file.read(4000);

        if(count>=upload_count)
        {
            UdpSocket.writeDatagram(line,remote_host,remote_port);
            return;
        }
        count++;
    }

    // 当上面没有return，说明所有数据已经发送完成，下面发送end请求
    QByteArray data2;
    QDataStream out2(&data2, QIODevice::WriteOnly);
    out2<<QString("upload file end")<<file_name<<upload_count;
    UdpSocket.writeDatagram(data2,remote_host,remote_port);
}

// 接收到上传回复
void MainWindow::Get_receive(QDataStream *in)
{
    QString file_name;
    int count;
    *in>>file_name;
    if(this->upload_file_path.endsWith(file_name))
    {
        *in>>count;
        // 服务器已经收到
        if(count == this->upload_count)
        {
            this->upload_count++;
            this->Upload_file();

            // 设置定时器
            upload_timer->stop();
            connect(upload_timer,SIGNAL(timeout()),this,SLOT(Upload_file()));
            upload_timer->start(1000);
        }
    }
}

// 收到上传文件接收完毕请求
void MainWindow::Get_upload_end(QDataStream *in)
{
    QString file_name;
    int count;
    *in>>file_name;
    if(this->upload_file_path.endsWith(file_name))
    {
        *in>>count;
        if(count == this->upload_count)
        {
            this->upload_count = -1;
            this->upload_timer->stop();
        }
    }
}
