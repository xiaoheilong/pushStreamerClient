#include "wsservicecloudgame.h"
//#include "easywsclient.hpp"
#include "json/json.h"
#include <QMessageBox>
//////////////WSCloudGameService
WSCloudGameService::WSCloudGameService(QObject * parent): QObject(parent),m_wsUrl(""), m_loginCommand("") {
    m_cloudGameIterator.reset();
    m_wsServiceThread = std::make_shared<WsServiceCloudGameThread>();
    connect(m_wsServiceThread.get() , SIGNAL(Parse(QString)) , this , SLOT(Parse(QString)));
}

WSCloudGameService::~WSCloudGameService(){
    //m_threadFlag = false;
    if(m_wsServiceThread.get()){
        disconnect(m_wsServiceThread.get() , SIGNAL(Parse(QString)) , this , SLOT(Parse(QString)));
    }
    if(m_cloudGameIterator.get() && m_wsServiceThread.get()){
        disconnect(m_cloudGameIterator.get(),SIGNAL(Send(QString)) , m_wsServiceThread.get() , SLOT(SendCallback(QString)));
    }
    m_wsServiceThread.reset();
    m_loginCommand.clear();
}

int WSCloudGameService::ConnectWS(QString  wsServerUrl , QString loginParams){
    m_wsUrl = wsServerUrl;
    m_loginCommand = loginParams;
    if(m_wsServiceThread){
        m_wsServiceThread->ConnectWS(m_wsUrl , m_loginCommand);
    }
    //start();
    //this->start();
    //QFuture<void>  func = QtConcurrent::run(this ,&WSCloudGameService::Parse);
    return 0;
}

int WSCloudGameService::DisconnectWS(){
    if(m_wsServiceThread){
        m_wsServiceThread->DisconnectWS();

    }
    return 0;
}

int WSCloudGameService::Send(QString msgData){
    if(m_wsServiceThread){
        //m_wsServiceThread->Send(msgData);
    }
    return 0 ;
}

void  WSCloudGameService::BindOutter(std::shared_ptr<CloudGameServiceIterator> iterator ){
//    if(m_cloudGameIterator.get() && m_wsServiceThread.get()){
//        disconnect(m_cloudGameIterator.get(),SIGNAL(Send(QString)) , m_wsServiceThread.get() , SLOT(SendCallback(QString)));
//    }
    m_cloudGameIterator = iterator;
    connect(m_cloudGameIterator.get(),SIGNAL(Send(QString)) , m_wsServiceThread.get() , SLOT(SendCallback(QString)));
}

void WSCloudGameService::Parse(QString data){
    if(data.isEmpty()){
        return ;
    }
    if(m_cloudGameIterator.get()){
        m_cloudGameIterator->ParseMessage(data);
    }
    return;
}
//////////////////////////////////////////////
