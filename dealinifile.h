#ifndef DEALINIFILE_H
#define DEALINIFILE_H


#include "mutexcellcallback.h"
#include <QSettings>
#include <memory>
#include <functional>
namespace DealIniFileSpace{
using namespace MutexCellCallbackSpace;

class DealIniFile
{
public:
    DealIniFile();
    ~DealIniFile();
    int OpenFile(QString filePath);
    QVariant GetValue(QString topic, QString key);  //直接定位目标块
    QVariant GetValue(QString key,QString keyValue , QString goalKey);  //不比对topic , 通过比对其下的key 来定位,

    bool  SetValue(QString topic , QString key , QString value);
    bool IsOpen();
private:
    QSettings *m_iniFileParse;
    std::shared_ptr<MutexCellCallback>  m_eventQueue;
    std::shared_ptr<std::mutex> m_mutex;
    QString m_iniFilePath;
    bool m_isOpen;
};
}
#endif // DEALINIFILE_H
