#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "processinfo.h"
#include<QTreeWidgetItem>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void startGetPcsInfo();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Ui::MainWindow *ui;

    ProcessInfo *processInfo;  //进程相关信息获取类
    QThread *processThread;    //信息获取子线程
    QList<QStringList*> *pInfoList;  //线程信息容器
    QList<QString*> *appInfoList;    //应用信息容器
    QVector<HICON> *hicon;       //应用图标容器

    void upDataGroupMem();      //更新物理内存信息
    void upDataGroupVM();       //更新虚拟内存信息

private slots:
    void on_killPro_clicked();   //结束进程
    void printProcessInfo(int cpuRate,int cntThread);   //重新渲染页面

    /*******菜单栏相关功能**************/
    void on_action_7_triggered();
    void on_action_triggered();
    void on_action_2_triggered(bool checked);
    void on_action_5_triggered();
    void on_action_6_triggered();
    void on_action_8_triggered();
    void on_action_3_triggered();
};


#endif // MAINWINDOW_H
