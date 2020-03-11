#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QDebug>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include<QTreeWidget>
#include <QThread>
#include <QMessageBox>
#include <Windows.h>

#pragma comment(lib,"Advapi32.lib")


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //公用信息容器的初始化
    pInfoList=new QList<QStringList*>();
    hicon=new QVector<HICON>;
    appInfoList=new QList<QString*>();

    //初始化信息获取类
    processInfo=new ProcessInfo(pInfoList,appInfoList,hicon);

    //开启子线程
    processThread=new QThread();
    processInfo->moveToThread(processThread);
    processThread->start();

    //绑定功能
    connect(this, SIGNAL(startGetPcsInfo()), processInfo, SLOT(getCurInfo()));
    connect(processInfo,SIGNAL(getInfoDone(int,int)),this,SLOT(printProcessInfo(int,int)));

    //向子线程发送信号
    emit startGetPcsInfo();

}

MainWindow::~MainWindow()
{
    delete ui;
}


//渲染界面信息
void MainWindow::printProcessInfo(int cpuRate,int cntThread){

    ui->proTable->clear();
    ui->userTable->clear();
    double memTotal=0;

    wchar_t strBuffer[256] ={0};
    DWORD dwSize = 256;
    GetUserName(strBuffer,&dwSize);



    QTreeWidgetItem *user=new QTreeWidgetItem(ui->userTable);

    user->setData(0,0,QString::fromWCharArray(strBuffer));
    user->setData(1,0,QString("活动"));

    QList<QTreeWidgetItem*> treeItemList;
    int len=pInfoList->size();
    for(int i=0;i<len;i++){

        //添加进程
        QTreeWidgetItem *temp=new QTreeWidgetItem(ui->proTable);
        temp->setData(0,0,QVariant(pInfoList->at(i)->at(0)));
        temp->setData(1,0,QVariant(pInfoList->at(i)->at(1).toInt()));
        temp->setData(2,0,QVariant(pInfoList->at(i)->at(2).toInt()));
        temp->setData(3,0,QVariant(pInfoList->at(i)->at(3)));
        temp->setData(4,0,QVariant(pInfoList->at(i)->at(4).toDouble()));

        temp->setBackgroundColor(1,QColor("#FFF4C4"));
        temp->setBackgroundColor(4,QColor("#FFF4C4"));

        //添加子进程
        for(int j=0;j<len;j++){
            if(pInfoList->at(j)->at(3)==pInfoList->at(i)->at(1)){
                QTreeWidgetItem *tempChild=new QTreeWidgetItem(temp);
                tempChild->setData(0,0,QVariant(pInfoList->at(j)->at(0)));
                tempChild->setData(1,0,QVariant(pInfoList->at(j)->at(1).toInt()));
                tempChild->setData(2,0,QVariant(pInfoList->at(j)->at(2).toInt()));
                tempChild->setData(3,0,QVariant(pInfoList->at(j)->at(3)));
                tempChild->setData(4,0,QVariant(pInfoList->at(j)->at(4).toDouble()));

                tempChild->setBackgroundColor(1,QColor("#FFF4C4"));
                tempChild->setBackgroundColor(4,QColor("#FFF4C4"));

                temp->addChild(tempChild);
            }
        }
        treeItemList.append(temp);

        //当前用户进程
        QTreeWidgetItem *usertemp=new QTreeWidgetItem(user);
        usertemp->setData(0,0,QVariant(pInfoList->at(i)->at(0)));
        usertemp->setData(1,0,QVariant(pInfoList->at(i)->at(1).toInt()));
        usertemp->setData(2,0,QVariant(pInfoList->at(i)->at(2).toInt()));
        usertemp->setData(3,0,QVariant(pInfoList->at(i)->at(3)));
        usertemp->setData(4,0,QVariant(pInfoList->at(i)->at(4).toDouble()));
        usertemp->setBackgroundColor(3,QColor("#FFF4C4"));
        usertemp->setBackgroundColor(4,QColor("#FFF4C4"));
        memTotal+=pInfoList->at(i)->at(4).toDouble();
        user->addChild(usertemp);
    }
    user->setData(4,0,QString("%1 MB").arg(memTotal));
    user->setData(5,0,QString("%1 %").arg(QString::number(cpuRate)));
    user->setBackgroundColor(4,QColor("#FFF4C4"));
    user->setBackgroundColor(5,QColor("#FFF4C4"));
    ui->userTable->addTopLevelItem(user);
    ui->proTable->addTopLevelItems(treeItemList);


    ui->label_14->setText(QString("%1 %").arg(QString::number(cpuRate)));
    ui->label_72->setText("线程                                " + QString::number(cntThread));
    ui->label_71->setText("进程                                " + QString::number(len));
    ui->proCounter->setText(QString("当前进程总数:%1").arg(QString::number(len)));
    ui->CPUCounter->setText(QString("CPU占用:%1 %").arg(QString::number(cpuRate)));

    ui->appTable->clear();
    // Sleep(500);
    QList<QTreeWidgetItem*> treeItemList2;
    int len2=appInfoList->size();
    for(int i=0;i<len2;i++){

        //添加进程
        QTreeWidgetItem *temp=new QTreeWidgetItem(ui->appTable);
        temp->setData(0,0,QVariant(*appInfoList->at(i)));

        treeItemList.append(temp);
    }
    ui->appTable->addTopLevelItems(treeItemList2);

    upDataGroupMem();

    upDataGroupVM();
}

//结束进程
void MainWindow::on_killPro_clicked()
{

    DWORD pid=ui->proTable->currentItem()->data(1,0).toUInt();

    HANDLE hYourTargetProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);

    TerminateProcess(hYourTargetProcess, 0);

}



/**************菜单栏*************/

//帮助菜单
void MainWindow::on_action_7_triggered()
{
    QMessageBox message(QMessageBox::Information,QString("About"),
                        QString("CUMTB Computer Science & Technology 2017 : "
                                "Yang Gao,Zhichen Zhang,Weikai Zeng,Zehua Fu,"
                                "Jiahao Lu"),QMessageBox::Yes|QMessageBox::No,nullptr);
    message.exec();
}

//退出
void MainWindow::on_action_triggered()
{
    this->close();
}

//置顶
void MainWindow::on_action_2_triggered(bool checkable)
{
    if(checkable == true){
        this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);//置顶
        this->show();
        checkable = false;
    }
    else if(checkable == false){
        this->setWindowFlags(this->windowFlags() & ~ Qt::WindowStaysOnTopHint);
        this->show();
        checkable = true;
    }
}

//关机
void MainWindow::on_action_5_triggered()
{
    system( "shutdown -p" );

}
//注销
void MainWindow::on_action_6_triggered()
{
    system( "shutdown -l" );

}
//重启
void MainWindow::on_action_8_triggered()
{
    system( "shutdown -r -t 0" );

}
//运行新任务
void MainWindow::on_action_3_triggered()
{
    QString processName = QFileDialog::getOpenFileName(
                this,
                tr("浏览"),QDir::currentPath(),tr("EXE (*.exe)"));
    STARTUPINFO startupInfo = {0};
    PROCESS_INFORMATION processInformation = { 0 };

    BOOL bSuccess = CreateProcess((LPWSTR)processName.utf16(), NULL, NULL, NULL, FALSE, NULL, NULL, NULL,&startupInfo,&processInformation);
}

/*************性能页面*************/
//物理内存
void MainWindow::upDataGroupMem(){
    MEMORYSTATUSEX state;
    state.dwLength = sizeof(state);
    GlobalMemoryStatusEx(&state);

    DWORDLONG Total_Phys = state.ullTotalPhys / (1024 * 1024);
    DWORDLONG Aviail_Phys = state.ullAvailPhys / (1024 * 1024);

    ui->label_51->setText("总数                        " + QString::number(Total_Phys));
    ui->label_52->setText("可用                        " + QString::number(Aviail_Phys));
    ui->label_53->setText("利用                        " + QString::number(state.dwMemoryLoad) + "%");
}
//虚拟内存
void MainWindow::upDataGroupVM(){
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    DWORDLONG Total_Virtual = statex.ullTotalPageFile / (1024 * 1024);
    DWORDLONG Aviail_Virtual = statex.ullAvailPageFile / (1024 * 1024);

    ui->label_61->setText("总数                                " + QString::number(Total_Virtual));
    ui->label_62->setText("可用                                " + QString::number(Aviail_Virtual));
}


