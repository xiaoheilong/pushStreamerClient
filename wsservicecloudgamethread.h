#ifndef WSSERVICECLOUDGAMETHREAD_H
#define WSSERVICECLOUDGAMETHREAD_H
#include "websocketConnection.h"
#include <QObject>
#include <QThread>
using namespace  WebSocketNamsSpace;
class WsServiceCloudGameThread : public QThread , public OutterInterfaceConnection
{
    Q_OBJECT
public:
    WsServiceCloudGameThread(QObject * parent = nullptr);
    ~WsServiceCloudGameThread();
protected:
    virtual void run();

    virtual void ConnectedCallback(std::string msg , int error) ;
    virtual void DisconnectedCallback(std::string msg , int error);
    virtual void MessageCallback(std::string msg , int error) ;
    virtual void FailureCallback(std::string msg , int error) ;
    virtual void InterruptCallback(std::string msg , int error) ;

public:
    virtual int ConnectWS(QString  wsServerUrl , QString loginParams);
    virtual int DisconnectWS();
    //virtual int Send(QString msgData);
signals:
    void Parse(QString data);
protected slots:
    void SendCallback(QString data);
private:
    //bool m_threadFlag;
    QString m_wsUrl ;
    QString m_loginCommand;
    //std::shared_ptr<WebSocket> m_wsSocket;
    std::shared_ptr<WsAppConnection> m_wsSocket;
    bool m_activeDisconnect;
};

#endif // WSSERVICECLOUDGAMETHREAD_H
