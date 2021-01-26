#include "wsservicecloudgamethread.h"


/// \brief CloudStreamer::CloudStreamer
#pragma comment( lib, "ws2_32" )
#include <QDebug>
#include <WinSock2.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <memory>
//#include "easywsclient.hpp"
#include <iostream>
//#include "CloudGame.h"
//#include "windows.h"


WsServiceCloudGameThread::WsServiceCloudGameThread(QObject * parent):QThread(parent),m_threadFlag(false) , m_wsUrl(""), m_loginCommand(""){
    m_wsSocket.reset();
}


WsServiceCloudGameThread::~WsServiceCloudGameThread(){
    DisconnectWS();
}

void WsServiceCloudGameThread::run(){
    m_threadFlag = true;
    m_wsSocket = std::shared_ptr<WebSocket>(WebSocket::from_url(m_wsUrl.toLocal8Bit().data()));
    if(!m_wsSocket){
         return;
    }
    if(!m_loginCommand.isEmpty()){
        std::string loginCommand = m_loginCommand.toStdString();
        m_wsSocket->send(loginCommand.c_str());
    }
    while (m_threadFlag) {
        if(!(m_wsSocket.get() && m_wsSocket->getReadyState() != WebSocket::CLOSED)){
            m_wsSocket = std::shared_ptr<WebSocket>(WebSocket::from_url(m_wsUrl.toLocal8Bit().data()));
            if(!m_wsSocket){
                 return;
            }
            if(!m_loginCommand.isEmpty()){
                std::string loginCommand = m_loginCommand.toStdString();
                m_wsSocket->send(loginCommand.c_str());
            }
        }
        WebSocket::pointer wsp = &*m_wsSocket; // <-- because a unique_ptr cannot be copied into a lambda
        m_wsSocket->poll();
        m_wsSocket->dispatch([&](const std::string & message) {
            //MessageBoxA(NULL, message.c_str() , "32323", MB_OK);
            if (message != "")
            {
                QString msg = message.c_str();
                emit Parse(msg);
            }
        });
    }
    // MessageBoxA(NULL, "socket close the connection one!" , "32323", MB_OK);
}

int WsServiceCloudGameThread::ConnectWS(QString  wsServerUrl , QString loginParams){
    m_wsUrl = wsServerUrl;
    m_loginCommand = loginParams;
    start();
    return 0;
}

int WsServiceCloudGameThread::DisconnectWS(){
    //MessageBoxA(NULL , "dasd" , "test" , MB_OK);
    m_threadFlag = false;
    if(m_wsSocket.get()){
        if(isRunning()){
            wait();
        }
        m_wsSocket.reset();
    }
    return 0;
}

void WsServiceCloudGameThread::SendCallback(QString msgData){
    if(m_wsSocket.get()){
        if(m_wsSocket->getReadyState() != WebSocket::CLOSED ){
            QString tempStr = "send message:" ;
            tempStr += msgData;
            std::string msg = msgData.toUtf8().data();//msgData.toLocal8Bit().data();//msgData.toStdString();
            m_wsSocket->send(msg.c_str());
        }else{
            //MessageBoxA(NULL, "socket close the connection!" , "niubi", MB_OK);
        }
    }
    return;
}

