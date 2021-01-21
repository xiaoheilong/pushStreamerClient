#include "cloudstreamer.h"
#include "wsservice.h"
#include "json/json.h"
#include <QApplication>
#include <QTextCodec>
#include <QMessageBox>
#include <windows.h>
#include <DbgHelp.h>
#include <QDate>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <minidumpapiset.h>
#include <iostream>
#include <QAbstractButton>
typedef CloudGameServiceIteratorSpace::CloudGameServiceIterator  CloudGameServiceIteratorEx;
using namespace WSServiceSpace;

//#include "shellapi.h"
//#include "shlwapi.h"
//#include   <fstream>
//#include  <TlHelp32.h>

//DWORD GetProcessidFromName(LPCTSTR name)
//{
//    PROCESSENTRY32 pe;
//    DWORD id = 0;
//    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//    pe.dwSize = sizeof(PROCESSENTRY32);
//    if (!Process32First(hSnapshot, &pe))
//        //LogEx(g_UI_ERROR, "GetProcessidFromName Process32First is failure!");
//    return 0;
//    while (1)
//    {
//        pe.dwSize = sizeof(PROCESSENTRY32);
//        if (Process32Next(hSnapshot, &pe) == FALSE) {
//            //LogEx(g_UI_ERROR, "GetProcessidFromName  result is false!");
//            break;
//        }
//        if (lstrcmp(pe.szExeFile, name) == 0)
//        {
//            id = pe.th32ProcessID;
//            //LogEx(g_UI_ERROR, "GetProcessidFromName  result is true!");
//            break;
//        }
//    }
//    CloseHandle(hSnapshot);
//    return id;
//}

//LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException){//程式异常捕获
//  EXCEPTION_RECORD* record = pException->ExceptionRecord;
//  QString errCode(QString::number(record->ExceptionCode,16)),errAdr(QString::number((uint)record->ExceptionAddress,16)),errMod;
////  QMessageBox::critical(NULL,"程式崩溃","<FONT size=4><div><b>发生错误</b><br/></div>"+
////      QString("<div>错误代码：%1</div><div>错误地址：%2</div></FONT>").arg(errCode).arg(errAdr),
////     QMessageBox::Ok);
//  QProcess::startDetached(qApp->applicationFilePath(), QStringList());
//  return EXCEPTION_EXECUTE_HANDLER;
// }
//异常捕获回调函数
LONG    errCallbackEx(EXCEPTION_POINTERS*  pException)
{
    QDir dir;
    dir.mkdir("./dumps");
    dir.cd("./dumps");

    /*
      ***保存数据代码***
    */

    QString fileName = dir.path() + "/" +
                       QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss.zzz") + ".dmp";

    LPCWSTR pFileName = (LPCWSTR)fileName.unicode();

    //创建 Dump 文件
    HANDLE hDumpFile = CreateFile(pFileName,
                                  GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    qDebug() << "create dumpFile:" << hDumpFile << INVALID_HANDLE_VALUE;
    if(hDumpFile != INVALID_HANDLE_VALUE)
    {
        //Dump信息
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;
        //写入Dump文件内容
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
    }

    EXCEPTION_RECORD* record = pException->ExceptionRecord;
    QString errCode(QString::number(record->ExceptionCode,16)),errAdr(QString::number((uint)record->ExceptionAddress,16)),errMod;


    qDebug() << "main thread:" << qApp->thread() << QThread::currentThread();

    int lastTime = 5;
    QTimer *timer = new QTimer();
    timer->setInterval(1000);
    QObject::connect(timer, &QTimer::timeout, [&lastTime, &errCode](){
        lastTime--;
        if(lastTime < 0)
        {
            //            qApp->quit();
            qDebug() << "start application:" << QProcess::startDetached(qApp->applicationFilePath(), QStringList());//重启
            return;
        }

        QString info = QString("未知原因崩溃，程序将在%1s后重启\r\n%2").arg(lastTime).arg(errCode);
    });
    timer->start();
    return EXCEPTION_EXECUTE_HANDLER;
}



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)(errCallbackEx));
    CloudStreamer w;
    //w.SetUIModel(UI_MODE::ONLY_PUSH_STREAMER);
    ///////
    //int rerun = GetProcessidFromName(L"gst-launch-1.0.exe");
    //////////////service bind
    std::shared_ptr<CloudGameServiceIteratorEx> cloudGameServiceIterator = std::make_shared<CloudGameServiceIteratorEx>();
    std::shared_ptr<WsServiceBase> wsServiceCloudGame(CreateCloudWSClient());
    if(!wsServiceCloudGame.get() || !cloudGameServiceIterator.get()){
        // QMessageBox::information(nullptr , "error!" , "wsServiceCloudGame or cloudGameServiceIterator shouldn't be empty!");
         return  -1;
    }
    wsServiceCloudGame->BindOutter(cloudGameServiceIterator);
    w.BindServiceIterator(cloudGameServiceIterator);
    ///中转服务器
    wsServiceCloudGame->ConnectWS("ws://socket1.cccsaas.com:9092" , "");//wss://101.132.169.20:4455/wss
    //wsServiceCloudGame->ConnectWS("wss://101.132.169.20:4455/wss" , "" );
    /////////////////
    w.show();
    //w.hide();
    int ret = a.exec();
    wsServiceCloudGame->DisconnectWS();
    if(ret == 777){
      QProcess::startDetached(qApp->applicationFilePath(), QStringList());
       return 0;
    }
    return ret ;
}
