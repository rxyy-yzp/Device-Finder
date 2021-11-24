#ifndef MY_SOCKET_H
#define MY_SOCKET_H

#include <vector>
#include <windows.h>
#include <QtNetwork/QUdpSocket>
#include <QMainWindow>
#include <fstream>
#include <QString>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace std;


//存放从recv_buff提取的目标
class Target{
public:
    Target(QHostAddress _ip, QString _name, quint16 _vendor, quint16 _proType,
           quint16 _proCode, QString _serNum) : ip(_ip), vendor(_vendor),
            productType(_proType), productCode(_proCode), deviceName(_name), serialNum(_serNum) {}

    QHostAddress getIp() const {return ip;}
    QString getDeviceName() const {return deviceName;}
    quint16 getVendor() const { return vendor;}
    quint16 getProductType() const { return productType;}
    quint16 getProductCode() const { return productCode;}
    QString getSerialNum() const { return serialNum;}
    bool operator<(const Target& t)const;
    friend ostream& operator<<(ostream& out, const Target& t);
private:
    QHostAddress ip;
    quint16 vendor;
    quint16 productType;
    quint16 productCode;
    QString deviceName;
    QString serialNum;
};

class SearchBase
{
public:
    friend class MainWindow;
    //三大功能search、recv、process
    virtual void Search() = 0;
    void recv(vector<Target*>&);
    bool Process(QByteArray &buf, QString &name, quint16 &vendor,
                 quint16 &proType, quint16 &proCode, QString &serNum);

    SearchBase(QHostAddress _host); //抽象基类不能实例化，但它的构造函数可以被基类调用
    virtual ~SearchBase() = default;

protected:
    QByteArray read_buf;
    QHostAddress host;  //绑定的ip地址
    QByteArray write_buf;
    QUdpSocket* my_socket;
};

class Mode1 : public SearchBase
{
public:
    using SearchBase::SearchBase;
    ~Mode1() {delete my_socket;}
    void Search();
};


class Mode2 : public SearchBase
{
public:
    Mode2(vector<QHostAddress> _Iprange, QHostAddress _host);
    ~Mode2() {delete my_socket;}
    void Search();
private:
    vector<QHostAddress> Iprange;
};

class Mode3 : public SearchBase
{
public:
    Mode3(quint32 _mask, quint32 _sub, QHostAddress _host);
    ~Mode3() {delete my_socket;}
    void Search();
private:
    quint32 sub_Address;        //为Remote_Ip 与 Mask与操作后的int数
    quint32 query_IP;           //要查询的IP，初始为sub_Address
    quint32 mask;               //给定的子网掩码
};

#endif // MY_SOCKET_H
