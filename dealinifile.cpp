#include "dealinifile.h"
#include "globaltools.h"
#include <QStringList>
#include <QFile>
namespace DealIniFileSpace{
DealIniFile::DealIniFile()
{
    m_iniFileParse.reset();
}

DealIniFile::~DealIniFile(){
    if(m_iniFileParse.get()){
        //m_iniFileParse->sync();
        m_iniFileParse.reset();
    }else{
        LOG_INFO("~DealIniFile  m_iniFileParse is NULL!");
    }
}


int DealIniFile::OpenFile(QString filePath){
    if(filePath.isEmpty()){
        return -1;
    }
    if(m_iniFileParse.get()){
        m_iniFileParse.reset();
    }
    if(!QFile::exists(filePath)){
        LOG_INFO(QString("DealIniFile::OpenFile %1 is not exist!").arg(filePath));
        return -1;
    }
    m_iniFileParse = std::make_shared<QSettings>( filePath, QSettings::Format::IniFormat);
    if(!m_iniFileParse.get()){
        return -1;
    }
    return 0;
}

QVariant DealIniFile::GetValue(QString topic, QString key){
    if(m_iniFileParse.get()){
        QString keyStr= topic + "/";
        keyStr += key;
        return m_iniFileParse->value(keyStr);
    }
    return QVariant();
}


QVariant DealIniFile::GetValue(QString key,QString keyValue , QString goalKey){
    if(!key.isEmpty() || !goalKey.isEmpty() ){
        if(m_iniFileParse.get()){
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
    if(!topic.isEmpty() && !key.isEmpty()){
        if(m_iniFileParse.get()){
            QString keyStr= topic + "/";
            keyStr += key;
            m_iniFileParse->setValue(keyStr , value.toLocal8Bit().data());
            //m_iniFileParse->sync();
            return true;
        }
    }
    return false;
}

}


