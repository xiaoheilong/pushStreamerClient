#ifndef WSSERVICECLOUDGAMETHREAD_H
#define WSSERVICECLOUDGAMETHREAD_H
#include "easywsclient.hpp"
#include <QObject>
#include <QThread>
using easywsclient::WebSocket;
class WsServiceCloudGameThread : public QThread
{
    Q_OBJECT
public:
    WsServiceCloudGameThread(QObject * parent = nullptr);
    ~WsServiceCloudGameThread();
protected:
    virtual void run();
public:
    virtual int ConnectWS(QString  wsServerUrl , QString loginParams);
    virtual int DisconnectWS();
    //virtual int Send(QString msgData);
signals:
    void Parse(QString data);
protected slots:
    void SendCallback(QString data);
private:
    bool m_threadFlag;
    QString m_wsUrl ;
    QString m_loginCommand;
    std::shared_ptr<WebSocket> m_wsSocket;
};

#endif // WSSERVICECLOUDGAMETHREAD_H
