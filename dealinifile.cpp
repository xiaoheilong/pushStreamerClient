#include "dealinifile.h"
#include "globaltools.h"
#include <QStringList>
#include <QFile>
namespace DealIniFileSpace{

#define DeletePtr(ptr)  if(ptr){  delete ptr; ptr = NULL; }

DealIniFile::DealIniFile():m_iniFileParse(NULL),m_eventQueue(NULL),m_mutex(NULL),m_iniFilePath(""),m_isOpen(false)
{
    m_mutex = std::make_shared<std::mutex>();
}

DealIniFile::~DealIniFile(){
    LOG_INFO(QString("DealIniFile is release file path:%1!").arg(m_iniFilePath));
    DeletePtr(m_iniFileParse);
    m_eventQueue.reset();
    m_mutex.reset();
    m_eventQueue = NULL;
    m_mutex = NULL;
}


int DealIniFile::OpenFile(QString filePath){
    m_iniFilePath = filePath;
    if(!m_eventQueue.get()){
        m_eventQueue = std::make_shared<MutexCellCallback>(filePath);
    }
    /////////
    MutexCallback * params = new MutexCallback();
    params->callback = std::make_shared<Function_Callback>([&](){
        if(filePath.isEmpty()){
            LOG_ERROR(QString("filePath:%1  path is empty!").arg(filePath))
            return;
        }
        if(m_iniFileParse){
            DeletePtr(m_iniFileParse);
        }
        if(!QFile::exists(filePath)){
            LOG_INFO(QString("DealIniFile::OpenFile %1 is not exist!").arg(filePath));
            return ;
        }
        if(!m_iniFileParse){
            m_iniFileParse = new QSettings( filePath, QSettings::Format::IniFormat);
            if(!m_iniFileParse){
                LOG_ERROR(QString("m_iniFileParse is null! the goal ini file path:%1").arg(filePath));
                return;
            }else{
                m_isOpen = true;
            }
        }
    });
    if(m_mutex.get()){
        m_mutex = std::make_shared<std::mutex>();
        params->mutex = m_mutex;
        params->validTime = 2000;
        m_eventQueue->PushCallback(std::shared_ptr<MutexCallback>(params));
    }
    return 0;
}

QVariant DealIniFile::GetValue(QString topic, QString key){
    if(m_iniFileParse){
        QString keyStr= topic + "/";
        keyStr += key;
        return m_iniFileParse->value(keyStr);
    }
    return QVariant();
}


QVariant DealIniFile::GetValue(QString key,QString keyValue , QString goalKey){
    if(!key.isEmpty() || !goalKey.isEmpty() ){
        if(m_iniFileParse){
             QStringList allKey = m_iniFileParse->childGroups();
             if(!allKey.isEmpty()){
                 for(int index = 0; index < allKey.size(); ++index){
                        //QString str = allKey[index];
                        QString itemId = GetValue(allKey[index] , key ).toString();
                        bool findFlag = 0 == keyValue.compare(itemId) ? true:false;
                        //bool testFlag = function(itemId , keyValue);
                        if(findFlag){
                            return GetValue(allKey[index] , goalKey);
                        }
                 }
             }
         }
    }
     return QVariant();
}


bool  DealIniFile::SetValue(QString topic , QString key , QString value){
    MutexCallback *params = new MutexCallback();
    params->callback = std::make_shared<Function_Callback>([&](){
        if(!m_iniFileParse){
            OpenFile(m_iniFilePath);
        }
        if(!topic.isEmpty() && !key.isEmpty()){
            if(m_iniFileParse){
                QString keyStr= topic + "/";
                keyStr += key;
                m_iniFileParse->setValue(keyStr , value.toLocal8Bit().data());
            }
        }
    });
    if(m_mutex.get()){
        params->mutex = m_mutex;
        params->validTime = 2000;
        m_eventQueue->PushCallback(std::shared_ptr<MutexCallback>(params));
    }
    return true;
}

bool DealIniFile::IsOpen(){
    return m_isOpen;
}

}


