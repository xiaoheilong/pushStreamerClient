#include "recordgameinfo.h"
#include "dealinifile.h"
#include <QCoreApplication>
namespace RecordGameInfoSpace{
using namespace DealIniFileSpace;
const QString g_pushStreamerRunningStatus = "/pushStreamerRunningStatus.ini";
const QString g_idName = "gameStatus";
const QString g_startGameParame = "startGameParame";
const QString g_startGameStatus = "startGameStatus";
const QString g_gameId="gameId";
const QString g_iniTopic = "pushStreamerInfo";
RecordGameInfo::RecordGameInfo()
{

}

RecordGameInfo::~RecordGameInfo(){

}

void RecordGameInfo::RecordInfo(QString startGameParame , QString gameId, int gameStatus){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
            streamConfig.SetValue(g_iniTopic,g_startGameParame ,startGameParame);
            streamConfig.SetValue(g_iniTopic,g_startGameStatus ,gameStatus ? "1":"0");
            streamConfig.SetValue(g_iniTopic,g_gameId ,gameId);
        }
    }
}

void RecordGameInfo::RecordInfo(int gameStatus){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
            streamConfig.SetValue(g_iniTopic,g_startGameStatus ,gameStatus ? "1":"0");
        }
    }
}

void RecordGameInfo::RecordInfo(QString gameId,int gameStatus){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
            streamConfig.SetValue(g_iniTopic,g_gameId ,gameId);
            streamConfig.SetValue(g_iniTopic,g_startGameStatus ,gameStatus ? "1":"0");
        }
    }
}

QString RecordGameInfo::GetGameInfo(){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
            return streamConfig.GetValue(g_iniTopic , g_startGameParame).toString();
        }
    }
    return "";
}

int RecordGameInfo::GetGameStatus(){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
            return streamConfig.GetValue(g_iniTopic , g_startGameStatus).toString().toInt();
        }
    }
    return 0;
}

QString GetGameId(){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
        if(0 == streamConfig.OpenFile(executePath + g_pushStreamerRunningStatus)){
            return streamConfig.GetValue(g_iniTopic , g_gameId).toString();
        }
    }
    return "";
}

}
