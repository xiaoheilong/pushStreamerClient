#include "recordgameinfo.h"
#include "dealinifile.h"
#include <QCoreApplication>
#include <chrono>
namespace RecordGameInfoSpace{
using namespace DealIniFileSpace;
const QString g_pushStreamerRunningStatus = "/pushStreamerRunningStatus.ini";
const QString g_idName = "gameStatus";
const QString g_startGameParame = "startGameParame";
const QString g_startGameStatus = "startGameStatus";
const QString g_gameId="gameId";
const QString g_iniTopic = "pushStreamerInfo";
const QString g_isUpdate = "isUpdate";

static std::mutex m_mutex;

RecordGameInfo * RecordGameInfo::m_instance = NULL;
RecordGameInfo::RecordGameInfo()
{

}

RecordGameInfo::~RecordGameInfo(){

}

void RecordGameInfo::ReleaseInstance(){
    if(m_instance){
        delete m_instance;
        m_instance = NULL;
    }
}

RecordGameInfo * RecordGameInfo::GetInstance(){
    if(!m_instance){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(!m_instance){
            m_instance = new RecordGameInfo();
        }
    }
    return m_instance;
}


void RecordGameInfo::RecordInfo(QString startGameParame , QString gameId, int gameStatus){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        std::unique_lock<std::mutex> lock(m_modifyMutex);
        /*if(lock.try_lock())*/{
            if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
                streamConfig.SetValue(g_iniTopic,g_startGameParame ,startGameParame);
                streamConfig.SetValue(g_iniTopic,g_startGameStatus ,gameStatus ? "1":"0");
                streamConfig.SetValue(g_iniTopic,g_gameId ,gameId);
            }
        }
    }
}

void RecordGameInfo::RecordInfo(QString startGameParame){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        std::unique_lock<std::mutex> lock(m_modifyMutex);
        /*if(lock.try_lock())*/{
            if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
                streamConfig.SetValue(g_iniTopic,g_startGameParame ,startGameParame);
            }
        }
    }
}

void RecordGameInfo::RecordInfo(int gameStatus){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
         std::unique_lock<std::mutex> lock(m_modifyMutex);
         /*if(lock.try_lock())*/{
             if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
                 streamConfig.SetValue(g_iniTopic,g_startGameStatus ,gameStatus ? "1":"0");
             }
         }
    }
}

void RecordGameInfo::RecordInfo(QString gameId,int gameStatus){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
         std::unique_lock<std::mutex> lock(m_modifyMutex);
         /*if(lock.try_lock())*/{
             if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
                 streamConfig.SetValue(g_iniTopic,g_gameId ,gameId);
                 streamConfig.SetValue(g_iniTopic,g_startGameStatus ,gameStatus ? "1":"0");
             }
         }
    }
}

QString RecordGameInfo::GetGameInfo(){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        std::unique_lock<std::mutex> lock(m_modifyMutex);
        /*if(lock.try_lock())*/{
            if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
                return streamConfig.GetValue(g_iniTopic , g_startGameParame).toString();
            }
        }
    }
    return "";
}

int RecordGameInfo::GetGameStatus(){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        std::unique_lock<std::mutex> lock(m_modifyMutex);
        /*if(lock.try_lock())*/{
            if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
                return streamConfig.GetValue(g_iniTopic , g_startGameStatus).toString().toInt();
            }
        }
    }
    return 0;
}

QString RecordGameInfo::GetGameId(){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        std::unique_lock<std::mutex> lock(m_modifyMutex);
       /* if(lock.try_lock())*/{
            if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
                return streamConfig.GetValue(g_iniTopic , g_gameId).toString();
            }
        }
    }
    return "";
}


bool RecordGameInfo::GetIsUpdate(){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        std::unique_lock<std::mutex> lock(m_modifyMutex);
        /*if(lock.try_lock())*/{
            if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
                if(0 == streamConfig.GetValue(g_iniTopic , g_isUpdate).toString().compare("1")){
                    return true;
                }
            }
        }
    }
    return false;
}

}
