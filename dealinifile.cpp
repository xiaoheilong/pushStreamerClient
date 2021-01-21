#include "dealinifile.h"
#include <QStringList>
namespace DealIniFileSpace{
DealIniFile::DealIniFile()
{
    m_iniFileParse.reset();
}

DealIniFile::~DealIniFile(){
    m_iniFileParse.reset();
}


int DealIniFile::OpenFile(QString filePath){
    if(filePath.isEmpty()){
        return -1;
    }
    if(m_iniFileParse.get()){
        m_iniFileParse.reset();
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

}


