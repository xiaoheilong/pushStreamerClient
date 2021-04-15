#include "cloudstreamer.h"
#include "wsservice.h"
#include "json/json.h"
#include "CloudGame.h"
#include "globaltools.h"
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
#include <tlhelp32.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
typedef CloudGameServiceIteratorSpace::CloudGameServiceIterator  CloudGameServiceIteratorEx;
using namespace WSServiceSpace;

std::shared_ptr<WsServiceBase> wsServiceCloudGame;

int GenerateMiniDump(PEXCEPTION_POINTERS pExceptionPointers)
{
    /////////
    QProcess p;
    QString c = "taskkill /f ";
    c += " /im";
    c += "gst-launch-1.0.exe";
    p.execute(c);
    p.close();

    if(KeyMouseIsValid()){
        KeyMouseClose();
    }

//    if(wsServiceCloudGame.get()){
//        wsServiceCloudGame->DisconnectWS();
//    }
    // 定义函数指针
    typedef BOOL(WINAPI * MiniDumpWriteDumpT)(
        HANDLE,
        DWORD,
        HANDLE,
        MINIDUMP_TYPE,
        PMINIDUMP_EXCEPTION_INFORMATION,
        PMINIDUMP_USER_STREAM_INFORMATION,
        PMINIDUMP_CALLBACK_INFORMATION
        );
    // 从 "DbgHelp.dll" 库中获取 "MiniDumpWriteDump" 函数
    MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;
    HMODULE hDbgHelp = LoadLibrary(L"DbgHelp.dll");
    if (NULL == hDbgHelp)
    {
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    pfnMiniDumpWriteDump = (MiniDumpWriteDumpT)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

    if (NULL == pfnMiniDumpWriteDump)
    {
        FreeLibrary(hDbgHelp);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    // 创建 dmp 文件件
    TCHAR szFileName[MAX_PATH] = {0};
    const TCHAR* szVersion = L"DumpDemo_v1.0";
    SYSTEMTIME stLocalTime;
    GetLocalTime(&stLocalTime);
    wsprintf(szFileName, L"%s-%04d%02d%02d-%02d%02d%02d.dmp",
        szVersion, stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
        stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond);
    HANDLE hDumpFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
    if (INVALID_HANDLE_VALUE == hDumpFile)
    {
        FreeLibrary(hDbgHelp);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    // 写入 dmp 文件
    MINIDUMP_EXCEPTION_INFORMATION expParam;
    expParam.ThreadId = GetCurrentThreadId();
    expParam.ExceptionPointers = pExceptionPointers;
    expParam.ClientPointers = FALSE;
    pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
        hDumpFile, MiniDumpWithDataSegs, (pExceptionPointers ? &expParam : NULL), NULL, NULL);
    // 释放文件
    CloseHandle(hDumpFile);
    FreeLibrary(hDbgHelp);
    //QProcess::startDetached(qApp->applicationFilePath(), QStringList());
    return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
    // 这里做一些异常的过滤或提示
    if (IsDebuggerPresent())
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    return GenerateMiniDump(lpExceptionInfo);
}


void StartProtectedScript(){
    ///////start protected bat
    QString appPath = QCoreApplication::applicationDirPath();
    QString scriptPath = appPath + "/protectCloudStreamer.bat";
    if(!scriptPath.isEmpty()){
        StartGame(scriptPath.toLocal8Bit().data() , NULL,SW_HIDE);
    }
}


int HasAnotherInstance(QString processName)
{
    int count = 0;
    PROCESSENTRY32 pe;
        DWORD id = 0;
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (!Process32First(hSnapshot, &pe))
            //LogEx(UI_ERROR , "GetProcessidFromName Process32First is failure!");
            return false;
        while (1)
        {
            if(count >= 2){
                break;
            }
            pe.dwSize = sizeof(PROCESSENTRY32);
            if (Process32Next(hSnapshot, &pe) == FALSE) {
                //LogEx(g_UI_ERROR, "GetProcessidFromName  result is false!");
                break;
            }
            std::string path1 = processName.toStdString();
            std::wstring path = s2ws(path1);
            if (lstrcmp(pe.szExeFile, path.c_str()) == 0)
            {
                ++count;
                //id = pe.th32ProcessID;
                //LogEx(g_UI_ERROR, "GetProcessidFromName  result is true!");
                //break;
            }
        }
        CloseHandle(hSnapshot);
        return count >= 2 ? true:false;
}


int main(int argc, char *argv[])
{
    if(HasAnotherInstance("CloudStreamer.exe")){
        //MessageBoxA(NULL, "CloudStreamer.exe is Already running!" , "error" ,MB_OK);
        return -1;
    }
    QApplication a(argc, argv);
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)(GenerateMiniDump));
    //set the log level according the  streamConfig.ini
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    if(0 != streamConfig.OpenFile(executePath + "//streamConfig.ini")){
        return -1;
    }
    QString logLevel = streamConfig.GetValue("streamConfig" , "logServel").toString();
    if(!logLevel.isEmpty()){
         int level = 0;
         level = logLevel.toInt();
         LOG_SET_LEVEL(level);
    }
    /////////
    CloudStreamer w;
    //w.SetUIModel(UI_MODE::ONLY_PUSH_STREAMER);
    //int rerun = GetProcessidFromName(L"gst-launch-1.0.exe");
    StartProtectedScript();
    //////////////service bind
    std::shared_ptr<CloudGameServiceIteratorEx> cloudGameServiceIterator = std::make_shared<CloudGameServiceIteratorEx>();
    wsServiceCloudGame  = std::shared_ptr<WsServiceBase>(CreateCloudWSClient());
    if(!wsServiceCloudGame.get() || !cloudGameServiceIterator.get()){
        // QMessageBox::information(nullptr , "error!" , "wsServiceCloudGame or cloudGameServiceIterator shouldn't be empty!");
         return  -1;
    }
    wsServiceCloudGame->BindOutter(cloudGameServiceIterator);
    w.BindServiceIterator(cloudGameServiceIterator);
    ///中转服务器
    QString  serverUrl = "ws://socket1.cccsaas.com:9092";
    //DealIniFile  streamConfig;
    QString serverUrl1 = streamConfig.GetValue("streamConfig" , "serverUrl").toString();
    if(!serverUrl1.isEmpty()){
         serverUrl = serverUrl1;
    }

    wsServiceCloudGame->ConnectWS(serverUrl , "");//wss://101.132.169.20:4455/wss
    //wsServiceCloudGame->ConnectWS("wss://101.132.169.20:4455/wss" , "" );
    /////////////////
    //w.show();
    w.hide();
    int ret = a.exec();
    wsServiceCloudGame->DisconnectWS();
    return ret ;
}
