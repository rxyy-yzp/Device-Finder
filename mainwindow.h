#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <QString>
#include <QListWidgetItem>
#include <QtNetwork/QUdpSocket>
#include <QMessageBox>
#include <QApplication>
#include "my_socket.h"
#include <QTreeWidget>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_add_btn_clicked();
    void on_remo_btn_clicked();
    void on_start_btn_clicked();
    void on_clear_btn_clicked();
    void on_stop_btn_clicked();
    void on_device_list_itemDoubleClicked(QTreeWidgetItem *item);

    void on_mode_currentTextChanged(const QString &arg1);



private:
    void free();    //释放item_vec内存
    void generate_add_list(vector<QHostAddress>&);//从加入列表中生成ip的vector
    int get_mode_code();    //得到下拉框中的询问方式
    void add_result_to_GUI(vector<Target*>&);
    void add_result_to_File(vector<Target*>& vec);
    bool get_judgedIP(int code, QHostAddress& ip);  //得到ip地址，如果ip不合法，返回0
    SearchBase* generate_mode();    //生成mode

    SearchBase* mode = nullptr; //mode对应后端的一个对象，实现前后端链接
    QTimer* timer;              //定时器，循环触发
    clock_t last_time = 0;           //用于gap自动调整
    double query_gap = 3;          //轮训间隔
    Ui::MainWindow *ui;
    QSet<QHostAddress> ip_set;  //用于判断p2p加入的ip地址是否重复
    vector<Target*> item_vec;   //用于存放接受到的设备信息
};
#endif // MAINWINDOW_H
