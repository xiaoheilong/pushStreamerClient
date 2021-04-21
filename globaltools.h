#ifndef GLOBALTOOLS_H
#define GLOBALTOOLS_H
#include "qlog4qt.h"
using namespace QLog4QtSpace;
////////////init the qtlog4
#if 1   // 写入日志系统
// 以文件行列记录信息，非以类名对象形式
#define LOG_DEBUG(msg) QLog::instance()->debug(QString("[%1] Message:%2").arg(QString("###FILE:%1  ###LINE:%2  ###FUNC:%3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)).arg(msg));

#define LOG_INFO(msg) QLog::instance()->info(QString("[%1] Message:%2").arg(QString("###FILE:%1  ###LINE:%2  ###FUNC:%3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)).arg(msg));

#define LOG_WARN(msg) QLog::instance()->warn(QString("[%1] Message:%2").arg(QString("###FILE:%1  ###LINE:%2  ###FUNC:%3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)).arg(msg));

#define LOG_ERROR(msg) QLog::instance()->error(QString("[%1] Message:%2").arg(QString("###FILE:%1  ###LINE:%2  ###FUNC:%3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)).arg(msg));

// 以类名对象形式记录信息
#define LOG_OBJECT_DEBUG(msg) QLog::instance()->debug(QString("[%1]%2").arg(QString("%1::%2:%3").arg(this->metaObject()->className()).arg(__FUNCTION__).arg(__LINE__)).arg(msg));

#define LOG_OBJECT_INFO(msg) QLog::instance()->info(msg);

#define LOG_OBJECT_WARN(msg) QLog::instance()->warn(msg);

#define LOG_OBJECT_ERROR(msg) QLog::instance()->error(msg);

#define LOG_SET_LEVEL(level)  QLog::instance()->setLevel(level);
#endif
//////////////some global function
int NSSleep(int intel);
///
#endif // GLOBALTOOLS_H
