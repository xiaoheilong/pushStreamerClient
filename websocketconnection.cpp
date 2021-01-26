﻿#include "websocketConnection.h"

namespace WebSocketNamsSpace{
WsAppConnection::WsAppConnection():thread_(NULL) , m_uri("") , m_isConnected(false), m_outter(NULL){

}

WsAppConnection::~WsAppConnection(){
    if(m_isConnected){
        close();
        terminate();
    }
}


void WsAppConnection::on_open(websocketpp::connection_hdl hdl)
{
    std::string msg = "hello";
    //c.send(hdl, msg, websocketpp::frame::opcode::text);
    c.get_alog().write(websocketpp::log::alevel::app, "Tx: " + msg);
    m_isConnected = true;
}

void WsAppConnection::on_message(websocketpp::connection_hdl hdl, message_ptr msg)
{
    std::cout << "on_message called with hdl: " << hdl.lock().get()
        << " and message: " << msg->get_payload()
        << std::endl;
    websocketpp::lib::error_code ec;
    if (ec)
    {
        std::cout << "Echo failed because " << ec.message() << std::endl;
    }
}

void WsAppConnection::on_close(websocketpp::connection_hdl hdl) {
    std::string message = "connection is close!";
    c.get_alog().write(websocketpp::log::alevel::app, "Tx: " + message);
    m_isConnected = false;
}

void WsAppConnection::on_failure(websocketpp::connection_hdl hdl) {
    std::string message = "connection is failure!";
    c.get_alog().write(websocketpp::log::alevel::app, "Tx: " + message);
    m_isConnected = false;
}

void WsAppConnection::on_pong(websocketpp::connection_hdl hdl , std::string msg) {
    std::string message = "";
    c.send(hdl, message, websocketpp::frame::opcode::PONG);
    c.get_alog().write(websocketpp::log::alevel::app, "recv ping: " + msg);
}

int  WsAppConnection::Send(std::string msg){
    if(!msg.empty()){
        if(m_isConnected){
            c.send(hdl_ ,msg , websocketpp::frame::opcode::text);
            c.get_alog().write(websocketpp::log::alevel::app, "send msg: " + msg);
            return 0;
        }
    }
    return -1;
}


void WsAppConnection::SetCallback(std::shared_ptr<OutterInterfaceConnection> outter){
    m_outter = outter;
}

void WsAppConnection::on_ping(websocketpp::connection_hdl hdl, std::string msg) {

}

void WsAppConnection::onTimer(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted)
        return;
    std::string msg = "{\"data\":{\"cpuUsage\":0.0,\"deviceNo\":\"c5ce4b4ce34d42c54eddb13042bf6851\",\"diskUsage\":91.66715240478516,\"gpuUsage\":3.0,\"internetIp\":\"219.136.204.62\",\"intranetIp\":\"192.168.50.11\",\"networkLatency\":37.0,\"networkOperator\":\"电信\",\"networkSpeed\":\"13.00\",\"ramUsage\":86.69999694824219,\"region\":\"中国广东省广州市\"},\"noncestr\":\"52298910\",\"sessionId\":\"1f0b48d8248cd2f3d72b4f9260901f42\",\"sign\":\"acc7738e67e43ca6e8942faea0db9c0c\",\"timestamp\":\"1611581648490\",\"type\":\"ReportDeviceState\"}";
    c.send(hdl_, msg, websocketpp::frame::opcode::text);
    c.get_alog().write(websocketpp::log::alevel::app, "send msg: " + msg);
    //m_timer->cancel();
   // m_timer = std::make_shared<boost::asio::deadline_timer>(c.get_io_service(), boost::posix_time::seconds(5));
   // m_timer->async_wait(bind(&WsAppConnection::onTimer, this, _1));
}

void WsAppConnection::On_Interrupt(websocketpp::connection_hdl hdl){
    std::string msg = "have a Interrupt!\n";
    c.get_alog().write(websocketpp::log::alevel::app,  msg);
    m_isConnected = false;
}

bool WsAppConnection::isConnected(){
    return m_isConnected;
}

int WsAppConnection::init(std::string wsUrl)
{
    if(wsUrl.empty()){
        return -1;
    }
    m_uri = wsUrl;
    c.set_access_channels(websocketpp::log::alevel::all);
    c.clear_access_channels(websocketpp::log::alevel::frame_payload);
    c.clear_access_channels(websocketpp::log::alevel::frame_header);
    c.init_asio();
    c.set_message_handler(websocketpp::lib::bind(&WsAppConnection::on_message, this, _1, _2));
    c.set_open_handler(websocketpp::lib::bind(&WsAppConnection::on_open, this, _1));
    c.set_close_handler(websocketpp::lib::bind(&WsAppConnection::on_close, this, _1));
    c.set_fail_handler(websocketpp::lib::bind(&WsAppConnection::on_failure, this, _1));
    c.set_pong_handler(websocketpp::lib::bind(&WsAppConnection::on_pong, this, _1, _2));
    c.set_interrupt_handler(websocketpp::lib::bind(&WsAppConnection::On_Interrupt, this, _1));
    //c.set_ping_handler(websocketpp::lib::bind(&WsAppConnection::on_ping, this, _1, _2));
    c.start_perpetual();
    ///timer
    //m_timer = std::make_shared<boost::asio::deadline_timer>(c.get_io_service(), boost::posix_time::seconds(10));
    //m_timer->async_wait(bind(&WsAppConnection::onTimer,this ,  _1));
    ///
    thread_ = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &c);
    return 0;
}

void WsAppConnection::connect()
{
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(m_uri, ec);
    if (ec)
    {
        std::cout << "could not create connection because: " << ec.message() << std::endl;
        return;
    }

    hdl_ = con->get_handle();
    c.connect(con);

}
void WsAppConnection::close()
{
    m_isConnected = false;
    c.close(hdl_, websocketpp::close::status::normal, "");
}

void WsAppConnection::terminate()
{
    c.stop_perpetual();
    if(thread_->joinable()){
        thread_->join();
    }
    m_isConnected = false;
}




}
