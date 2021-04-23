#include "recordgameinfo.h"
#include "dealinifile.h"
#include "globaltools.h"
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
RecordGameInfo::RecordGameInfo():m_dealIniFile(NULL)
{

}

RecordGameInfo::~RecordGameInfo(){
    if(m_dealIniFile.get()){
        m_dealIniFile.reset();
    }
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


void RecordGameInfo::InitDealIniFile(){
    if(!m_dealIniFile.get()){
        m_dealIniFile = std::make_shared<DealIniFile>();
        if(m_dealIniFile.get()){
            QString executePath = QCoreApplication::applicationDirPath();
            if(!executePath.isEmpty()){
                executePath += g_pushStreamerRunningStatus;
                if(0 != m_dealIniFile->OpenFile(executePath)){
                    LOG_ERROR(QString("the ini file :%1 is open failure!").arg(executePath));
                }else{
                     LOG_INFO(QString("the ini file :%1 is open success!").arg(executePath));
                }
            }
        }
    }
}

void RecordGameInfo::RecordInfo(QString startGameParame , QString gameId, int gameStatus)
{
    InitDealIniFile();
    if(m_dealIniFile.get()){
        m_dealIniFile->SetValue(g_iniTopic,g_startGameParame ,startGameParame);
        m_dealIniFile->SetValue(g_iniTopic,g_startGameStatus ,gameStatus ? "1":"0");
        m_dealIniFile->SetValue(g_iniTopic,g_gameId ,gameId);
    }
}

void RecordGameInfo::RecordInfo(QString startGameParame){
    InitDealIniFile();
    if(m_dealIniFile.get()){
        m_dealIniFile->SetValue(g_iniTopic,g_startGameParame ,startGameParame);
    }
}

void RecordGameInfo::RecordInfo(int gameStatus){
    InitDealIniFile();
    if(m_dealIniFile.get()){
        m_dealIniFile->SetValue(g_iniTopic,g_startGameStatus ,gameStatus ? "1":"0");
    }
}

void RecordGameInfo::RecordInfo(QString gameId,int gameStatus){
    InitDealIniFile();
    if(m_dealIniFile.get()){
        m_dealIniFile->SetValue(g_iniTopic,g_gameId ,gameId);
        m_dealIniFile->SetValue(g_iniTopic,g_startGameStatus ,gameStatus ? "1":"0");
    }
}

QString RecordGameInfo::GetGameInfo(){
    InitDealIniFile();
    if(m_dealIniFile.get()){
        return m_dealIniFile->GetValue(g_iniTopic , g_startGameParame).toString();
    }
    return "";
}

int RecordGameInfo::GetGameStatus(){
    InitDealIniFile();
    if(m_dealIniFile.get()){
        return m_dealIniFile->GetValue(g_iniTopic , g_startGameStatus).toString().toInt();
    }
    return 0;
}

QString RecordGameInfo::GetGameId(){
    InitDealIniFile();
    if(m_dealIniFile.get()){
        return m_dealIniFile->GetValue(g_iniTopic , g_gameId).toString();
    }
    return "";
}


bool RecordGameInfo::GetIsUpdate(){
    InitDealIniFile();
    if(m_dealIniFile.get()){
        if(0 == m_dealIniFile->GetValue(g_iniTopic , g_isUpdate).toString().compare("1")){
            return true;
        }
    }
    return false;
}

}
