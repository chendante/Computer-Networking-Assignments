#include "myftp.h"

myftp::myftp(QHostAddress other_host, int other_port, MainWindow *parent)
{
    this->pp = parent;
    this->client_host = other_host;
    this->client_port = other_port;

    this->receiver_count = -1;
}

bool myftp::Is_equal(QHostAddress other_host, int other_port)
{
    if(this->client_host.isEqual(other_host)&&other_port==client_port)
    {
        return true;
    }
    else{
        return false;
    }
}

// 发送信息
void myftp::SendMessage(QByteArray data)
{
    UdpSocket.writeDatagram(data,client_host, client_port);
}

// 发送目录
void myftp::SendDir()
{
    QString file_path = this->pp->GetFilePath();
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QDir dir(file_path);
    // 当填写目录不存在时，不发送任何内容
    if(!dir.exists())
    {
        this->pp->InsertRecord("wrong path\n");
        return;
    }

    // 将选中目录内容取出
    dir.setFilter(QDir::Files|QDir::Hidden|QDir::NoSymLinks);
    dir.setSorting(QDir::Time);
    QFileInfoList d_list = dir.entryInfoList();
    qDebug()<<d_list.size();

    //将d_list 中内容整理为JSON格式
    QJsonArray json_array;
    for (int i = 0; i < d_list.size(); i++)
    {
        QFileInfo fileInfo = d_list.at(i);
        QJsonObject file_json;
        file_json.insert("name",fileInfo.fileName());
        file_json.insert("time",fileInfo.created().toString("yyyy-MM-dd hh:mm:ss"));
        file_json.insert("size",QString::number(fileInfo.size()));
        file_json.insert("path",fileInfo.absoluteFilePath());
        json_array.append(QJsonValue(file_json));
    }
    QJsonDocument json_doc;
    json_doc.setArray(json_array);
    out<<QString("send dir")<<json_doc.toJson(QJsonDocument::Compact);
    this->SendMessage(data);
}

// 发送文件
void myftp::SendFile(QString file_name, int want_count)
{
    QString file_path = this->pp->GetFilePath()+file_name;
    QFile file;
    file.setFileName(file_path);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"wrong" ;
        return;
    }

    // 发送对应文件片
    int count=0;
    while(!file.atEnd()){
        QByteArray line;
        QDataStream out3(&line, QIODevice::WriteOnly);
        out3<<QString("download file data")<<file_name<<count<<file.read(4000);

        if(count>=want_count)
        {
            this->SendMessage(line);
            return;
        }
        count++;
    }

    // 当请求数大于文件总数时，发送已经发完信息
    QByteArray data2;
    QDataStream out2(&data2, QIODevice::WriteOnly);
    out2<<QString("download file end")<<file_name<<count;
    this->SendMessage(data2);
}

void myftp::ReceiveFile(QDataStream *in)
{
    QString file_name;
    *in>>file_name;
    int count;
    *in>>count;
    qDebug()<<count<<endl;
    // 当是一个发送的开始时，对其进行初始化
    if(receiver_count == -1 && count == 0)
    {
        receiver_count = 0;
        receiver_file_name = file_name;
    }
    if(file_name == this->receiver_file_name)
    {
        // 如果获取的文件片是当前想要的文件片则处理
        if(count == receiver_count)
        {
            receiver_count++;
            QByteArray datagram;
            *in>>datagram;
            this->receiver_file_data.append(datagram.data());
        }
    }
    // 发送已经收到的请求
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out<<QString("upload file receive")<<receiver_file_name<<receiver_count-1;
    this->SendMessage(data);
}

void myftp::ReceiveEnd(QDataStream *in)
{
    QString file_name;
    *in>>file_name;
    int count;
    *in>>count;
    if(file_name == this->receiver_file_name)
    {
        // 如果获取的文件片是当前想要的文件片则处理
        if(count == receiver_count)
        {
            // 下载结束保存文件
            QFile file(this->pp->GetFilePath()+receiver_file_name);
            if(!file.open(QIODevice::WriteOnly)) return;
            file.write(receiver_file_data,receiver_file_data.size());
            file.close();

            // 发送已经收到的请求
            QByteArray data;
            QDataStream out(&data, QIODevice::WriteOnly);
            out<<QString("upload file end")<<receiver_file_name<<receiver_count;
            this->SendMessage(data);

            // 将相关设置初始化
            receiver_count = -1;
            receiver_file_data.clear();
        }
    }
}
