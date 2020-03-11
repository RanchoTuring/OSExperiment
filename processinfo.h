#ifndef PROCESSINFO_H
#define PROCESSINFO_H

#include <QObject>
#include <QTimer>
#include <Windows.h>



class ProcessInfo:public QObject
{
    Q_OBJECT

signals:
    void getInfoDone(int cpu,int cnt);

public:
    ProcessInfo(QList<QStringList*> *p,QList<QString*> *a,QVector<HICON> *h);
    ~ProcessInfo();
        LONGLONG CalculaterInterval(FILETIME time1, FILETIME time2);
        int GetCPURate();
    QList<QStringList*> *pInfoList;

    static QList<QString*> *aInfoList;
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
static QVector<HICON> *hicon;
   // static QList<QString*> str;

private slots:
    void getCurInfo();
private:
    QTimer *qtimer;
};
//QList<QString*> ProcessInfo::str=QList<QString*>();



#endif // PROCESSINFO_H

