#ifndef WSSERVICE_H
#define WSSERVICE_H
#include "cloudgameserviceiterator.h"
#include <memory>
//#include <QObject>
////////wsService f
namespace WSServiceSpace{
//using namespace CloudGameServiceIteratorSpace;
class WsServiceBase
{
public:
    WsServiceBase();
    virtual ~WsServiceBase();
protected:
    typedef CloudGameServiceIteratorSpace::CloudGameServiceIterator CloudGameServiceIterator;
public:
    virtual int ConnectWS(QString  wsServerUrl , QString loginParams) = 0;
    virtual int DisconnectWS() = 0;
    virtual int Send(QString msgData) = 0;
    virtual void BindOutter(std::shared_ptr<CloudGameServiceIterator> iterator ) = 0;
protected:
    virtual void Parse(QString data) = 0;
};


WsServiceBase * CreateCloudWSClient();
void ReleseCloudWSClient(WsServiceBase * service);
}
#endif // WSSERVICE_H
