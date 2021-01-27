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
    std::string msg = "websocket connected!";
    //c.send(hdl, msg, websocketpp::frame::opcode::text);
    c.get_alog().write(websocketpp::log::alevel::app, "Tx: " + msg);
    m_isConnected = true;
    if(m_outter)
    {
        m_outter->ConnectedCallback(msg , 0);
    }
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

    if(m_outter)
    {
        std::string message = msg->get_payload();
        m_outter->MessageCallback(message, 0);
    }
}

void WsAppConnection::on_close(websocketpp::connection_hdl hdl) {
    std::string message = "connection is close!";
    c.get_alog().write(websocketpp::log::alevel::app, "Tx: " + message);
    m_isConnected = false;
    if(m_outter)
    {
        m_outter->DisconnectedCallback(" websocket disconnect!", 0);
    }
}

void WsAppConnection::on_failure(websocketpp::connection_hdl hdl) {
    std::string message = "connection is failure!";
    c.get_alog().write(websocketpp::log::alevel::app, "Tx: " + message);
    m_isConnected = false;
    if(m_outter)
    {
        m_outter->FailureCallback("websocket failure!", 0);
    }
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


void WsAppConnection::SetCallback(OutterInterfaceConnection *outter){
    m_outter = outter;
}

void WsAppConnection::on_ping(websocketpp::connection_hdl hdl, std::string msg) {

}

void WsAppConnection::onTimer(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted)
        return;
    std::string msg = "{\"deviceNo\":\"c5ce4b4ce34d42c54eddb13042bf6851\" , \"gameId\":\"gmly\" ,\"status\":0,\"type\":\"GameReportDeviceState\"}";
    c.send(hdl_, msg, websocketpp::frame::opcode::text);
    c.get_alog().write(websocketpp::log::alevel::app, "send msg: " + msg);
    m_timer->cancel();
    m_timer = std::make_shared<boost::asio::deadline_timer>(c.get_io_service(), boost::posix_time::seconds(5));
    m_timer->async_wait(bind(&WsAppConnection::onTimer, this, _1));
}

void WsAppConnection::On_Interrupt(websocketpp::connection_hdl hdl){
    std::string msg = "have a Interrupt!\n";
    c.get_alog().write(websocketpp::log::alevel::app,  msg);
    m_isConnected = false;
    if(m_outter)
    {
        m_outter->InterruptCallback("websocket Interrupt!", 0);
    }
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
//    m_timer = std::make_shared<boost::asio::deadline_timer>(c.get_io_service(), boost::posix_time::seconds(5));
//    m_timer->async_wait(bind(&WsAppConnection::onTimer,this ,  _1));
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
    c.stop();
    c.stop_perpetual();
    thread_->join();
    m_isConnected = false;
}




}
