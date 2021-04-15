#include "qlog4qt.h"
//#include <QApplication>
#include <QCoreApplication>
#include "log4qt/propertyconfigurator.h"
#include "globaltools.h"
namespace QLog4QtSpace{
using namespace Log4Qt;
QAtomicPointer<QLog> QLog::_instance = 0;
QMutex QLog::_mutex;

QLog::QLog(QObject *parent) : QObject(parent)
{
    QString configpath = QCoreApplication::applicationDirPath() +"/log4qt.conf";//QApplication::applicationDirPath()+"/log4qt.conf";
    Log4Qt::PropertyConfigurator::configure(configpath);
    logger()->info("start used log!!!");
}


QLog *QLog::instance()
{
#ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE
    if(!QAtomicPointer::isTestAndSetNative())//运行时检测
        qDebug() << "Error: TestAndSetNative not supported!";
#endif
    //使用双重检测。
    /*! testAndSetOrders操作保证在原子操作前和后的的内存访问
     * 不会被重新排序。
     */
    if(_instance.testAndSetOrdered(0, 0))//第一次检测
    {
        QMutexLocker locker(&_mutex);//加互斥锁。

        _instance.testAndSetOrdered(0, new QLog);//第二次检测。
    }
    return _instance;
}


void QLog::setLevel(int level){
//    NULL_INT = 0,
//    ALL_INT = 32,
//    TRACE_INT = 64,
//    DEBUG_INT = 96,
//    INFO_INT = 128,
//    WARN_INT = 150,
//    ERROR_INT = 182,
//    FATAL_INT = 214,
//    OFF_INT = 255
    Level levelReal = Level::NULL_INT;
    switch(level){
    case 0:{
        levelReal = Level::NULL_INT;
    }
        break;
    case 1:{
        levelReal = Level::ALL_INT;
    }
        break;
    case 2:{
        levelReal = Level::TRACE_INT;
    }
        break;
    case 3:{
        levelReal = Level::DEBUG_INT;
    }
        break;
    case 4:{
        levelReal = Level::INFO_INT;
    }
        break;
    case 5:{
        levelReal = Level::WARN_INT;
    }
        break;
    case 6:{
        levelReal = Level::ERROR_INT;
    }
        break;
    case 7:{
        levelReal = Level::FATAL_INT;
    }
        break;
    case 8:{
        levelReal = Level::OFF_INT;
    }
        break;
    default:{

    }
        break;
    }
    logger()->setLevel(levelReal);
    LOG_ERROR(QString("set log level:%1").arg(level));
}

}
