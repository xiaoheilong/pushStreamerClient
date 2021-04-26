#ifndef DEALINIFILE_H
#define DEALINIFILE_H

#include <QSettings>
#include <memory>
#include <functional>
namespace DealIniFileSpace{
class DealIniFile
{
public:
    DealIniFile();
    ~DealIniFile();
    int OpenFile(QString filePath);
    QVariant GetValue(QString topic, QString key);  //直接定位目标块
    QVariant GetValue(QString key,QString keyValue , QString goalKey);  //不比对topic , 通过比对其下的key 来定位,

    bool  SetValue(QString topic , QString key , QString value);
    QSettings * GetInitFileParse();
private:
    QSettings *m_iniFileParse;
};
}
#endif // DEALINIFILE_H
