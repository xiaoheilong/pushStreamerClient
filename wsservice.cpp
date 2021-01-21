#include "wsservice.h"
#include "wsservicecloudgame.h"
namespace WSServiceSpace{
WsServiceBase::WsServiceBase()
{

}

WsServiceBase::~WsServiceBase(){

}


WsServiceBase * CreateCloudWSClient(){
    return new WSCloudGameService();
}

void ReleseCloudWSClient(WsServiceBase * service){
    if(service){
        delete service;
        service = nullptr;
    }
}
}
