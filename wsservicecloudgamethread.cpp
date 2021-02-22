#include "wsservicecloudgamethread.h"


/// \brief CloudStreamer::CloudStreamer
#pragma comment( lib, "ws2_32" )
#include <QDebug>
#include <WinSock2.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <memory>
#include <iostream>
//#include "CloudGame.h"
//#include "windows.h"


WsServiceCloudGameThread::WsServiceCloudGameThread(QObject * parent):QThread(parent), m_wsUrl(""), m_loginCommand(""),m_activeDisconnect(false){
    m_wsSocket.reset();
    connect(this , SIGNAL(AccidentalSignal()) , this , SLOT(AccidentalTermination()));
}


WsServiceCloudGameThread::~WsServiceCloudGameThread(){
    DisconnectWS();
    disconnect(this , SIGNAL(AccidentalSignal()) , this , SLOT(AccidentalTermination()));
}

void WsServiceCloudGameThread::ConnectedCallback(std::string msg , int error) {
    m_activeDisconnect = false;
}

void WsServiceCloudGameThread::DisconnectedCallback(std::string msg , int error){
    emit AccidentalSignal();
}

void WsServiceCloudGameThread::MessageCallback(std::string msg , int error) {
    if (msg != "")
    {
        QString message = msg.c_str();
        emit Parse(message);
    }
}

void WsServiceCloudGameThread::FailureCallback(std::string msg , int error) {
    emit AccidentalSignal();
}

void WsServiceCloudGameThread::InterruptCallback(std::string msg , int error) {
   emit AccidentalSignal();
}

void WsServiceCloudGameThread::run(){
    m_wsSocket = std::make_shared<WsAppConnection>();
    m_wsSocket->init(m_wsUrl.toStdString());
    m_wsSocket->connect();
    m_wsSocket->SetCallback(this);
    this->exec();
}

int WsServiceCloudGameThread::ConnectWS(QString  wsServerUrl , QString loginParams){
    m_wsUrl = wsServerUrl;
    m_loginCommand = loginParams;
    //DisconnectWS();
    start();
    return 0;
}

int WsServiceCloudGameThread::DisconnectWS(){
    m_activeDisconnect = true;
    if(isRunning()){
        this->quit();
        this->wait();
    }
    if(m_wsSocket){
        //m_wsSocket->terminate();
        m_wsSocket->close();
        m_wsSocket.reset();
        m_wsSocket = nullptr;
    }
    return 0;
}

void WsServiceCloudGameThread::AccidentalTermination(){
    if(isRunning()){
        this->quit();
        this->wait();
    }
    if(m_wsSocket){
        m_wsSocket->closeByAccident();
        m_wsSocket.reset();
        m_wsSocket = nullptr;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if(!m_activeDisconnect){
        ConnectWS(m_wsUrl , m_loginCommand);
    }
}

void WsServiceCloudGameThread::SendCallback(QString msgData){
    if(m_wsSocket.get()){
        std::string msg = msgData.toUtf8().data();
        m_wsSocket->Send(msg.c_str());
    }
    return;
}

