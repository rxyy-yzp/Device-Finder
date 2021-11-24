#include "my_socket.h"

bool Target::operator<(const Target& t)const
{
    return ip.toIPv4Address() < t.ip.toIPv4Address();
}
ostream& operator<<(ostream& out, const Target& t)
{

    QString info="";
    info += "ip address: " + t.ip.toString() + "\n";
    info += "name: " + t.deviceName + "\n";
    info += "vendor: "+ QString::number(t.vendor) + "\n";
    info += "product code: " + QString::number(t.productCode) + "\n";
    info += "product type: " + QString::number(t.productType) + "\n";
    info += "serial number: " + t.serialNum + "\n\n\n";
    out << info.toStdString() << endl;
}
SearchBase::SearchBase(QHostAddress _host) : host(_host)
{
    my_socket = new QUdpSocket();
    my_socket->bind(host ,44818);
    write_buf.resize(24);
    for(int i = 0; i < 24; ++i)
        write_buf[i] = 0;   //总共 is 24 bytes
     write_buf[0] = 0x63;   //第一位为63h，唯一的标识
}

//响应信息解析函数
bool SearchBase::Process(QByteArray &buf, QString &name, quint16 &vendor,
             quint16 &proType, quint16 &proCode, QString &serNum)
{
    if(buf.size() != 86 || buf[0] != 'c')
        return false;

    QByteArray a;
    name = buf.mid(63, 22).data();

    //------解析vendor-----
    a = buf.mid(48, 2);
    reverse(a.begin(), a.end());
    vendor = a.toHex().toInt(NULL,16);

    //------解析protype-----
    a = buf.mid(50, 2);
    reverse(a.begin(), a.end());
    proType = a.toHex().toInt(NULL,16);

    //------解析procode-----
    a = buf.mid(52, 2);
    reverse(a.begin(), a.end());
    proCode = a.toHex().toInt(NULL,16);

    //------解析sernum-----
    a = buf.mid(58, 4);
    reverse(a.begin(), a.end());
    serNum = a.toHex();

    return true;
}
//接受消息函数
void SearchBase::recv(vector<Target*>& vec)
{
    if (!my_socket->hasPendingDatagrams())
        Sleep(3000);
    while(my_socket->hasPendingDatagrams())
    {
        QHostAddress ip;
        quint16 port, vendor, proType, proCode;
        QString name, serNum;

        //接受数据报
        read_buf.resize(my_socket->pendingDatagramSize());
        my_socket->readDatagram(read_buf.data(), read_buf.size(), &ip, &port);

        //进行消息解析
        if(Process(read_buf, name, vendor, proType, proCode, serNum))
        {
            vec.push_back(new Target(ip, name, vendor, proType, proCode, serNum));
        }
        if (!my_socket->hasPendingDatagrams())
            Sleep(200);
    }
    sort(vec.begin(), vec.end(), [=](Target* a, Target* b){return *a < *b;});
}

void Mode1::Search()
{

    my_socket->writeDatagram(write_buf.data(), 1024, QHostAddress(QString("255.255.255.255")), 44818);
}

Mode2::Mode2(vector<QHostAddress> _Iprange, QHostAddress _host)
    : SearchBase(_host), Iprange(_Iprange) {}

void Mode2::Search()
{

    for (auto it : Iprange)
        my_socket->writeDatagram(write_buf.data(), 1024, it, 44818);
}



Mode3::Mode3(quint32 _mask, quint32 _sub, QHostAddress _host)
    : SearchBase(_host), sub_Address(_sub), query_IP(_sub), mask(_mask){}

void Mode3::Search()
{
    int i = 0;
    while ((query_IP & mask) == sub_Address && i < 2000)
    {
        my_socket->writeDatagram(write_buf.data(), 1024, QHostAddress(query_IP), 44818);
        query_IP++;
        i++;
    }
}
