#include "processinfo.h"

#pragma comment(lib,"psapi.lib")
#pragma  comment(lib, "user32.lib")
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>
#include <QDebug>


QList<QString*>* ProcessInfo::aInfoList=nullptr;
QVector<HICON>* ProcessInfo::hicon=nullptr;


ProcessInfo::ProcessInfo(QList<QStringList*> *p,QList<QString*> *a,QVector<HICON> *h)
{
    pInfoList=p;
    aInfoList=a;
    hicon=h;
    //定时器
    qtimer=new QTimer();
    qtimer->start(3000);
    //定时器与功能绑定
    connect(qtimer,SIGNAL(timeout()),this,SLOT(getCurInfo()));
}


ProcessInfo::~ProcessInfo()
{
	delete qtimer;
}


//获取应用窗口
BOOL ProcessInfo::EnumWindowsProc(
		HWND hwnd,
		LPARAM lParam)
{
	wchar_t caption[200];
	memset(caption, 0, sizeof(caption));
	if ( GetParent(hwnd)==nullptr  &&  IsWindowVisible(hwnd) ){
		::GetWindowText(hwnd, caption, 200);
		QString* temp=new QString(QString::fromWCharArray(caption));
		if(*temp!=""){
			aInfoList->append(temp);
			int GCL_HICON;
			if (GetClassLong(hwnd, GCL_HICON))
			{
				HICON hIcon =(HICON)GetClassLong(hwnd, GCL_HICON);
				if(hIcon)
				{
					hicon->push_back(hIcon);
				}
			}
		}
	}
	return TRUE;
}


//获取进程相关信息
void ProcessInfo::getCurInfo()
{
    //while(1){
    pInfoList->clear();
    aInfoList->clear();
    PROCESSENTRY32 pe32;
    // 在使用这个结构之前，先设置它的大小
    pe32.dwSize = sizeof(pe32);
    // 给系统内的所有进程拍一个快照
    HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hProcessSnap == INVALID_HANDLE_VALUE) {
        printf(" CreateToolhelp32Snapshot调用失败！ \n");
    }
    // 遍历进程快照，轮流显示每个进程的信息
    int cntThread=0;
    BOOL bMore = ::Process32First(hProcessSnap, &pe32);

    //遍历进程快照
    while(bMore) {

        HANDLE hProcess =OpenProcess(PROCESS_ALL_ACCESS,FALSE,pe32.th32ProcessID);
        cntThread+=pe32.cntThreads;
        //内存
        PROCESS_MEMORY_COUNTERS pmc={0};
        ::GetProcessMemoryInfo(hProcess,&pmc,sizeof(pmc));

        QStringList *temp=new QStringList(QString("%1").arg(pe32.szExeFile));
        *temp<<QString("%1").arg(pe32.th32ProcessID)
            <<QString("%1").arg(pe32.pcPriClassBase)
           <<QString("%1").arg(pe32.th32ParentProcessID)
          <<QString::asprintf("%.1f",pmc.WorkingSetSize/1000/1000.0);
        pInfoList->append(temp);
        bMore = ::Process32Next(hProcessSnap, &pe32);
    }

    // 清除snapshot对象
    ::CloseHandle(hProcessSnap);

    //获取应用信息
    EnumWindows(EnumWindowsProc,reinterpret_cast<LPARAM> (this));

    //向主线程发送信号
    emit getInfoDone(GetCPURate(),cntThread);

}


//CPU时间计算
LONGLONG ProcessInfo::CalculaterInterval(FILETIME time1, FILETIME time2)
{
    LARGE_INTEGER uliTime1;
    LARGE_INTEGER uliTime2;

    uliTime1.LowPart = time1.dwLowDateTime;
    uliTime1.HighPart = time1.dwHighDateTime;

    uliTime2.LowPart = time2.dwLowDateTime;
    uliTime2.HighPart = time2.dwHighDateTime;

    return uliTime1.QuadPart - uliTime2.QuadPart;
}


//CPU利用率
int ProcessInfo::GetCPURate()
{

    FILETIME idleTime;//空闲时间
    FILETIME kernelTime;//核心态时间
    FILETIME userTime;//用户态时间

    FILETIME preidleTime;
    FILETIME prekernelTime;
    FILETIME preuserTime;

    int cpurate;

    bool bRet = GetSystemTimes(&idleTime, &kernelTime, &userTime);
    if (!bRet) {
        return bRet;
    }

    preidleTime = idleTime;
    prekernelTime = kernelTime;
    preuserTime = userTime;

    Sleep(1000);    //间隔1S

    bRet = GetSystemTimes(&idleTime, &kernelTime, &userTime);
    if (!bRet) {
        return bRet;
    }

    LONGLONG idle = CalculaterInterval(idleTime, preidleTime);
    LONGLONG kernel = CalculaterInterval(kernelTime, prekernelTime);
    LONGLONG user = CalculaterInterval(userTime, preuserTime);

    //CPU利用率 = （总时间 - 空闲时间）/ 总时间
    cpurate = ((kernel - idle + user) * 100) / (kernel + user);
//qDebug()<<cpurate<<endl;
    return cpurate;
}
