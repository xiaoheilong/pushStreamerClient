#ifndef WSSERVICECLOUDGAME_H
#define WSSERVICECLOUDGAME_H

#include "wsservice.h"
#include "wsservicecloudgamethread.h"

using namespace WSServiceSpace;
class WSCloudGameService:public QObject ,public WsServiceBase
{
     Q_OBJECT
public:
    WSCloudGameService(QObject * parent = nullptr);
    ~WSCloudGameService();
public:
    virtual int ConnectWS(QString  wsServerUrl , QString loginParams);
    virtual int DisconnectWS();
    virtual int Send(QString msgData);
    virtual void BindOutter(std::shared_ptr<CloudGameServiceIterator> iterator );
public slots:
    void Parse(QString data);
private:
    QString m_wsUrl ;
    QString m_loginCommand;
    std::shared_ptr<CloudGameServiceIterator> m_cloudGameIterator;
    std::shared_ptr<WsServiceCloudGameThread> m_wsServiceThread;
};

#endif // WSSERVICECLOUDGAME_H
