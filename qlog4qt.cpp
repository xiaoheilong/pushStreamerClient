#include "qlog4qt.h"
//#include <QApplication>
#include <QCoreApplication>
#include "log4qt/propertyconfigurator.h"
#include "globaltools.h"
namespace QLog4QtSpace{

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

}
