#include "dealinifile.h"
#include "globaltools.h"
#include <QStringList>
#include <QFile>
namespace DealIniFileSpace{

#define DeletePtr(ptr)  if(ptr){  delete ptr; ptr = NULL; }


DealIniFile::DealIniFile():m_iniFileParse(NULL)
{
}

DealIniFile::~DealIniFile(){
    DeletePtr(m_iniFileParse);
}


int DealIniFile::OpenFile(QString filePath){
    if(filePath.isEmpty()){
        return -1;
    }
    DeletePtr(m_iniFileParse);
    if(!QFile::exists(filePath)){
        LOG_INFO(QString("DealIniFile::OpenFile %1 is not exist!").arg(filePath));
        return -1;
    }
    m_iniFileParse = new QSettings( filePath, QSettings::Format::IniFormat);
    if(!m_iniFileParse){
        return -1;
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
    if(!topic.isEmpty() && !key.isEmpty()){
        if(m_iniFileParse){
            QString keyStr= topic + "/";
            keyStr += key;
            m_iniFileParse->setValue(keyStr , value.toLocal8Bit().data());
            return true;
        }
    }
    return false;
}

}


