﻿#include "wsservicecloudgamethread.h"


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
}


WsServiceCloudGameThread::~WsServiceCloudGameThread(){
    DisconnectWS();
}

void WsServiceCloudGameThread::ConnectedCallback(std::string msg , int error) {
    m_activeDisconnect = false;
}

void WsServiceCloudGameThread::DisconnectedCallback(std::string msg , int error){
    if(!m_activeDisconnect){
        ConnectWS(m_wsUrl , m_loginCommand);
    }
}

void WsServiceCloudGameThread::MessageCallback(std::string msg , int error) {
    if (msg != "")
    {
        QString message = msg.c_str();
        emit Parse(message);
    }
}

void WsServiceCloudGameThread::FailureCallback(std::string msg , int error) {
    if(!m_activeDisconnect){
        ConnectWS(m_wsUrl , m_loginCommand);
    }
}

void WsServiceCloudGameThread::InterruptCallback(std::string msg , int error) {
    if(!m_activeDisconnect){
        ConnectWS(m_wsUrl , m_loginCommand);
    }
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
    start();
    return 0;
}

int WsServiceCloudGameThread::DisconnectWS(){
    m_activeDisconnect = true;
    if(isRunning()){
        this->quit();
        this->wait();
    }
    if(m_wsSocket->isConnected()){
        m_wsSocket->terminate();
        m_wsSocket->close();
    }
    return 0;
}

void WsServiceCloudGameThread::SendCallback(QString msgData){
    if(m_wsSocket.get()){
        if(m_wsSocket->isConnected() ){
            std::string msg = msgData.toUtf8().data();
            m_wsSocket->Send(msg.c_str());
        }else{

        }
    }
    return;
}

