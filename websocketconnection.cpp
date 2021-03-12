#include "websocketConnection.h"

namespace WebSocketNamsSpace{
WsAppConnection::WsAppConnection():thread_(NULL) , m_uri("") , m_isConnected(false), m_outter(NULL){
    hdl_.reset();
    thread_.reset();

    c.set_access_channels(websocketpp::log::alevel::all);
    c.clear_access_channels(websocketpp::log::alevel::frame_payload);
    c.clear_access_channels(websocketpp::log::alevel::frame_header);
    c.init_asio();
    c.start_perpetual();
    thread_ = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &c);
}

WsAppConnection::~WsAppConnection(){
    try{
        c.stop();
        c.stop_perpetual();
        close();
        if(thread_.get()){
            thread_->join();
            thread_.reset();
        }
    }catch(websocketpp::exception const & e){
        int a =0;
        a++;
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
    //m_isConnected = false;
    if(m_outter)
    {
        m_outter->DisconnectedCallback(" websocket disconnect!", 0);
    }
}

void WsAppConnection::on_failure(websocketpp::connection_hdl hdl) {
    std::string message = "connection is failure!";
    c.get_alog().write(websocketpp::log::alevel::app, "Tx: " + message);
    //m_isConnected = false;
    if(m_outter)
    {
        m_outter->FailureCallback("websocket failure!", 0);
    }
}

void WsAppConnection::on_pong(websocketpp::connection_hdl hdl , std::string msg) {
    std::string message = "";
    //c.send(hdl, message, websocketpp::frame::opcode::PONG);
    c.get_alog().write(websocketpp::log::alevel::app, "recv ping: " + msg);
}


// UTF8转std:string
// 转换过程：先将utf8转双字节Unicode编码，再通过WideCharToMultiByte将宽字符转换为多字节。
std::string UTF8_To_string(const std::string& str)
{
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* pwBuf = new wchar_t[nwLen + 1];    //一定要加1，不然会出现尾巴
    memset(pwBuf, 0, nwLen * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
    char* pBuf = new char[nLen + 1];
    memset(pBuf, 0, nLen + 1);
    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string strRet = pBuf;

    delete []pBuf;
    delete []pwBuf;
    pBuf = NULL;
    pwBuf = NULL;

    return strRet;
}

// std:string转UTF8
std::string string_To_UTF8(const std::string& str)
{
    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    wchar_t* pwBuf = new wchar_t[nwLen + 1];    //一定要加1，不然会出现尾巴
    ZeroMemory(pwBuf, nwLen * 2 + 2);
    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
    char* pBuf = new char[nLen + 1];
    ZeroMemory(pBuf, nLen + 1);
    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string strRet(pBuf);

    delete []pwBuf;
    delete []pBuf;
    pwBuf = NULL;
    pBuf  = NULL;

    return strRet;
}

int  WsAppConnection::Send(std::string msg , OpcodeValue opcode1){
    if(!msg.empty()){
        if(m_isConnected){
            std::string data = string_To_UTF8(msg);
            if(!data.empty())
            {
                try{
                    c.send(hdl_ ,data , opcode1);
                    c.get_alog().write(websocketpp::log::alevel::app, "send msg: " + data);
                }catch(websocketpp::exception const & e){
                     c.get_alog().write(websocketpp::log::alevel::fail, "send msg failure: " + data);
                }
            }
            return 0;
        }
    }
    return -1;
}


void WsAppConnection::SetCallback(OutterInterfaceConnection *outter){
    m_outter = outter;
}

bool WsAppConnection::on_ping(websocketpp::connection_hdl hdl, std::string msg) {
    //Send("test", websocketpp::frame::opcode::pong);
    return true;
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
    //m_isConnected = false;
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
    c.set_message_handler(websocketpp::lib::bind(&WsAppConnection::on_message, this, _1, _2));
    c.set_open_handler(websocketpp::lib::bind(&WsAppConnection::on_open, this, _1));
    c.set_close_handler(websocketpp::lib::bind(&WsAppConnection::on_close, this, _1));
    c.set_fail_handler(websocketpp::lib::bind(&WsAppConnection::on_failure, this, _1));
    c.set_pong_handler(websocketpp::lib::bind(&WsAppConnection::on_pong, this, _1, _2));
    c.set_interrupt_handler(websocketpp::lib::bind(&WsAppConnection::On_Interrupt, this, _1));
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
    if(m_isConnected){
        try{
            c.close(hdl_, websocketpp::close::status::normal, "");
        }catch(websocketpp::exception const & e){
            int a =0;
            a++;
        }
    }
    m_isConnected = false;
}

void WsAppConnection::closeByAccident(){
    if(m_isConnected){
        try{
            c.close(hdl_, websocketpp::close::status::normal, "");
        }catch(websocketpp::exception const & e){
            int a=0;
            a++;
        }
    }
    m_isConnected = false;
}

void WsAppConnection::terminate()
{
    return;
}




}
