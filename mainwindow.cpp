#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <windows.h>
#include <ctime>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    timer=new QTimer(this);
    ui->setupUi(this);

    ui->gap_line->setValidator(new QDoubleValidator(2, 1000, 2, this));
    ui->p2p_addr0->setEnabled(0);ui->p2p_addr1->setEnabled(0);
    ui->p2p_addr2->setEnabled(0);ui->p2p_addr3->setEnabled(0);
    ui->remote_ip0->setEnabled(0);ui->remote_ip1->setEnabled(0);
    ui->remote_ip2->setEnabled(0);ui->remote_ip3->setEnabled(0);
    ui->mask_ip0->setEnabled(0);ui->mask_ip1->setEnabled(0);
    ui->mask_ip2->setEnabled(0);ui->mask_ip3->setEnabled(0);

    connect(timer, SIGNAL(timeout()), this, SLOT(on_start_btn_clicked()));

    //startTimer(1000);
}

enum{SRC_IP, REMOTE_IP, P2P_IP, MASK};
enum{LOCAL, P2P, REMOTE};

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
    free();
}
//从添加列表中得到ip保存到vector
void MainWindow::generate_add_list(vector<QHostAddress>& vec)
{
    int row=0;
    QString line;
    while(row < ui->ip_list->count())
    {
        line = ui->ip_list->item(row)->text();
        vec.push_back(QHostAddress(line));
        row++;
    }
}
//得到合法的ip地址
bool MainWindow::get_judgedIP(int code, QHostAddress& ip)
{
    //从四个框中获得ip字符串，保存到vec中
    QVector<QString> vec_ip;
    if (code == SRC_IP)
    {
        vec_ip.push_back(ui->src_ip0->text());
        vec_ip.push_back(ui->src_ip1->text());
        vec_ip.push_back(ui->src_ip2->text());
        vec_ip.push_back(ui->src_ip3->text());
    }
    else if (code == P2P_IP)
    {
        vec_ip.push_back(ui->p2p_addr0->text());
        vec_ip.push_back(ui->p2p_addr1->text());
        vec_ip.push_back(ui->p2p_addr2->text());
        vec_ip.push_back(ui->p2p_addr3->text());
    }
    else if (code == REMOTE_IP)
    {
        vec_ip.push_back(ui->remote_ip0->text());
        vec_ip.push_back(ui->remote_ip1->text());
        vec_ip.push_back(ui->remote_ip2->text());
        vec_ip.push_back(ui->remote_ip3->text());
    }
    else if (code == MASK)
    {
        vec_ip.push_back(ui->mask_ip0->text());
        vec_ip.push_back(ui->mask_ip1->text());
        vec_ip.push_back(ui->mask_ip2->text());
        vec_ip.push_back(ui->mask_ip3->text());
    }
    else
    {
        QMessageBox::critical(this,"错误信息","发生严重错误，程序返回");
        exit(0);
    }
    //把vec中的ip段整合起来
    QString temp;
    for (int i = 0; i < 4; i++)
    {
        for (auto it : vec_ip[i])
        {
            if (it < '0' || it >'9')
            {
                QMessageBox::critical(this,"错误信息","IP地址不合法，请检查！");
                return 0;
            }
        }
        if (vec_ip[i].toInt() < 0 || vec_ip[i].toInt() > 255)
        {
            QMessageBox::critical(this,"错误信息","IP地址不合法，请检查！");
            return 0;
        }
        temp += vec_ip[i];
        if (i != 3)
            temp += ".";
    }
    ip = QHostAddress(temp);
    if (ip.toIPv4Address() == 0)
    {
        QMessageBox::critical(this,"错误信息","未设定IP地址，请检查！");
        return 0;
    }
    return 1;
}

int MainWindow::get_mode_code()
{
    QString temp = ui->mode->currentText();
    if (temp[0] == 'L')
        return LOCAL;
    else if (temp[0] == 'P')
        return P2P;
    else if (temp[0] == 'R')
        return REMOTE;
    else
        return -1;
}

SearchBase* MainWindow::generate_mode()
{
    int code = get_mode_code();
    if (code == -1)
        return nullptr;
    SearchBase *mode;
    QHostAddress host_ip;
    //得到的ip地址不合法
    if (get_judgedIP(SRC_IP, host_ip) == 0)
        return nullptr;

    if(code == LOCAL)
    {
        mode = new Mode1(host_ip);
    }
    else if(code == P2P)
    {
        vector<QHostAddress> p2p_list;
        generate_add_list(p2p_list);
        //如果p2p查询没有指定查询地址，同样报错
        if (p2p_list.empty())
            return nullptr;
        mode = new Mode2(p2p_list, host_ip);
    }
    else if (code == REMOTE)
    {
        QHostAddress mask, remote_IP;
        get_judgedIP(REMOTE_IP, remote_IP);
        get_judgedIP(MASK, mask);
        quint32 _sub = remote_IP.toIPv4Address() & mask.toIPv4Address();
        mode = new Mode3(mask.toIPv4Address(), _sub, host_ip);
    }
    return mode;
}
void MainWindow::on_add_btn_clicked()
{
    QVector<QString> vec_ip;
    vec_ip.push_back(ui->p2p_addr0->text());
    vec_ip.push_back(ui->p2p_addr1->text());
    vec_ip.push_back(ui->p2p_addr2->text());
    vec_ip.push_back(ui->p2p_addr3->text());

    QHostAddress ip;
    if(ui->mode->currentText()[0] != 'P' || get_judgedIP(P2P_IP, ip) == 0)
        return;
    if (ip_set.find(ip) == ip_set.end())
    {
        ip_set.insert(ip);
        ui->ip_list->addItem(ip.toString());
    }
    else
    {
        QMessageBox::critical(this,"错误信息","IP地址已在列表中");
        return;
    }
    return;
}

void MainWindow::on_remo_btn_clicked()
{
    //ip_list为当前的点对点功能中ip添加列表
    int row = ui->ip_list->currentRow();
    if(row < 0)
    {
        QMessageBox::critical(this,"错误信息","未选中删除项目");
        return;
    }

    QString ip = ui->ip_list->item(row)->text();
    ui->ip_list->takeItem(row);
    ip_set.erase(ip_set.find(QHostAddress(ip)));
    return;
}

void MainWindow::on_start_btn_clicked()
{

    query_gap = ui->gap_line->text().toDouble();

    mode = generate_mode();
    //没有得到合适的mode
    if (mode == nullptr)
    {
        QMessageBox::critical(this,"错误信息","选择的查询方式不合法或查询列表为空，程序已退出！");
        return;
    }
    ui->status->setText("on running");
    ui->gap_line->setEnabled(0);
    ui->mode->setEnabled(0);

    if (clock() - last_time > query_gap * 1000 + 2000)
    {
        qDebug() << clock() - last_time << '\n';
        query_gap += 3;
        ui->gap_line->setText(QString::number(query_gap, 'f', 2));
    }
    else
    {
        if (query_gap > 2)
            query_gap -= 1;
        ui->gap_line->setText(QString::number(query_gap, 'f', 2));
    }
    last_time = clock();
    free(); //free放在后面影响查看设备信息
    ui->device_list->clear();
    mode->Search();
    Sleep(10);
    mode->recv(item_vec);
    add_result_to_GUI(item_vec);
    add_result_to_File(item_vec);

    if(timer->isActive() == false)
        timer->start(query_gap * 1000);
    delete mode;
}
void MainWindow::add_result_to_File(vector<Target*>& vec)
{
    ofstream f("out.txt");
    for (auto it : vec)
        f << *it;
}
void MainWindow::add_result_to_GUI(vector<Target*>& vec)
{
    ui->device_num->setText(QString::number(item_vec.size()));

    QList<QTreeWidgetItem *> rootList;
    //添加第一个父节点

    QTreeWidgetItem *rootItem1 = new QTreeWidgetItem;
    QIcon icon = QIcon(":link.ico");
    rootItem1->setIcon(0, icon);
    ui->device_list->setIconSize(QSize(55, 24));
    rootItem1->setText(0,tr("LAPTOP-CGU18OPD"));
    rootList.append(rootItem1);
    for(auto it : vec)
    {
        //添加子节点
        QTreeWidgetItem *Item1 = new QTreeWidgetItem(rootItem1, QStringList(it->getIp().toString()+','+it->getDeviceName()));        QIcon icon = QIcon(":Terminal.ico");
        Item1->setIcon(0, icon);
        ui->device_list->setIconSize(QSize(40, 14));
        rootItem1->addChild(Item1);
    }
    ui->device_list->insertTopLevelItems(0, rootList);  //将结点插入部件中
    ui->device_list->expandAll(); //全部展开
}


void MainWindow::on_clear_btn_clicked()
{
    ui->device_num->setText(QString::number(0));
    ui->device_list->clear();
}

void MainWindow::on_stop_btn_clicked()
{
    timer->stop();
    ui->mode->setEnabled(1);
    ui->gap_line->setEnabled(1);
    ui->status->setText("Browse has been stopped");
    return;
}

void MainWindow::free()
{
    for (auto it : item_vec)
        delete it;
    item_vec.clear();
}

void MainWindow::on_device_list_itemDoubleClicked(QTreeWidgetItem *item)
{
    auto it = item_vec.begin();
    for (; it != item_vec.end() && ((*it)->getIp()).toString() +',' + (*it)->getDeviceName() != item->text(0); it++);
    if (it == item_vec.end())
        return;
    QString info="";
    info += "name: " + (*it)->getDeviceName() + "\n";
    info += "ip address: " + (*it)->getIp().toString() + "\n";
    info += "vendor: "+ QString::number((*it)->getVendor()) + "\n";
    info += "product code: " + QString::number((*it)->getProductCode()) + "\n";
    info += "product type: " + QString::number((*it)->getProductType()) + "\n";
    info += "serial number: " + (*it)->getSerialNum() + "\n";
    QMessageBox::information(this, "device info", info);
}


void MainWindow::on_mode_currentTextChanged(const QString &arg1)
{
    if (arg1[0] == 'L')
    {
        ui->p2p_addr0->setEnabled(0);
        ui->p2p_addr1->setEnabled(0);
        ui->p2p_addr2->setEnabled(0);
        ui->p2p_addr3->setEnabled(0);

        ui->remote_ip0->setEnabled(0);
        ui->remote_ip1->setEnabled(0);
        ui->remote_ip2->setEnabled(0);
        ui->remote_ip3->setEnabled(0);

        ui->mask_ip0->setEnabled(0);
        ui->mask_ip1->setEnabled(0);
        ui->mask_ip2->setEnabled(0);
        ui->mask_ip3->setEnabled(0);

    }
   else if (arg1[0] == 'P')
    {
        ui->p2p_addr0->setEnabled(1);
        ui->p2p_addr1->setEnabled(1);
        ui->p2p_addr2->setEnabled(1);
        ui->p2p_addr3->setEnabled(1);

        ui->remote_ip0->setEnabled(0);
        ui->remote_ip1->setEnabled(0);
        ui->remote_ip2->setEnabled(0);
        ui->remote_ip3->setEnabled(0);

        ui->mask_ip0->setEnabled(0);
        ui->mask_ip1->setEnabled(0);
        ui->mask_ip2->setEnabled(0);
        ui->mask_ip3->setEnabled(0);
    }
    else
    {
        ui->p2p_addr0->setEnabled(0);
        ui->p2p_addr1->setEnabled(0);
        ui->p2p_addr2->setEnabled(0);
        ui->p2p_addr3->setEnabled(0);

        ui->remote_ip0->setEnabled(1);
        ui->remote_ip1->setEnabled(1);
        ui->remote_ip2->setEnabled(1);
        ui->remote_ip3->setEnabled(1);

        ui->mask_ip0->setEnabled(1);
        ui->mask_ip1->setEnabled(1);
        ui->mask_ip2->setEnabled(1);
        ui->mask_ip3->setEnabled(1);
    }
}



