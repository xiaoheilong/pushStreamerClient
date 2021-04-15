#ifndef QLOG4QT_H
#define QLOG4QT_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include "log4qt/logger.h"
#include "log4qt/log4qt.h"

namespace QLog4QtSpace{

    class QLog : public QObject
    {
        Q_OBJECT
        LOG4QT_DECLARE_QCLASS_LOGGER

    public:
        explicit QLog(QObject *parent = 0);
        static QLog * instance();

        virtual void debug(const QString& log){logger()->debug(log);}
        virtual void info (const QString& log){logger()->info(log);}
        virtual void warn (const QString& log){logger()->warn(log);}
        virtual void error(const QString& log){logger()->error(log);}
        virtual void fatal(const QString& log){logger()->fatal(log);}

        virtual void setLevel(int level);
    private:
        static QAtomicPointer<QLog> _instance;
        static QMutex _mutex;
    };

}
#endif // QLOG4QT_H
