#include "websocketConnection.h"
#include "globaltools.h"
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
    LOG_INFO(QString("WsAppConnection::WsAppConnection()"));
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
        LOG_INFO(QString("WsAppConnection::~WsAppConnection release occur failure!"));
    }
    LOG_INFO(QString("WsAppConnection::~WsAppConnection()"));
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
    LOG_INFO(QString(" %1 on_open: %2").arg(m_uri.c_str()).arg(msg.c_str()));
}

void WsAppConnection::on_message(websocketpp::connection_hdl hdl, message_ptr msg)
{
    LOG_INFO(QString("on_message called with and message: %1  from %2!").arg( msg->get_payload().c_str()).arg(m_uri.c_str()));
    websocketpp::lib::error_code ec;
    if (ec)
    {
        //std::cout << "Echo failed because " << ec.message() << std::endl;
        LOG_INFO(QString("Echo failed because %1").arg(ec.message().c_str()));
    }

    if(m_outter)
    {
        std::string message = msg->get_payload();
        m_outter->MessageCallback(message, 0);
    }
}

void WsAppConnection::on_close(websocketpp::connection_hdl hdl) {
    std::string message = "connection is close!";
    LOG_INFO(QString("%1 from %2!").arg(message.c_str()).arg(m_uri.c_str()));
    c.get_alog().write(websocketpp::log::alevel::app, "Tx: " + message);
    //m_isConnected = false;
    if(m_outter)
    {
        m_outter->DisconnectedCallback(" websocket disconnect!", 0);
    }
}

void WsAppConnection::on_failure(websocketpp::connection_hdl hdl) {
    std::string message = "connection is failure!";
    LOG_INFO(QString("%1 from %2!").arg(message.c_str()).arg(m_uri.c_str()));
    c.get_alog().write(websocketpp::log::alevel::app, "Tx: " + message);
    //m_isConnected = false;
    if(m_outter)
    {
        m_outter->FailureCallback("websocket failure!", 0);
    }
}

void WsAppConnection::on_pong(websocketpp::connection_hdl hdl , std::string msg) {
    std::string message = "on_pong";
    LOG_INFO(QString("on_pong msg:%1 from %2!").arg(msg.c_str()).arg(m_uri.c_str()));
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
    LOG_INFO(QString("Send msg failure to %1!").arg(m_uri.c_str()));
    return -1;
}


void WsAppConnection::SetCallback(OutterInterfaceConnection *outter){
    m_outter = outter;
}

bool WsAppConnection::on_ping(websocketpp::connection_hdl hdl, std::string msg) {
    //Send("test", websocketpp::frame::opcode::pong);
    LOG_INFO(QString("on_ping from %1!").arg(m_uri.c_str()));
    return true;
}

void WsAppConnection::onTimer(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::operation_aborted){
        LOG_INFO(QString("WsAppConnection::onTimer occur boost::asio::error::operation_aborted wsServerUrl:%1"));
        return;
    }
    std::string msg = "{\"deviceNo\":\"c5ce4b4ce34d42c54eddb13042bf6851\" , \"gameId\":\"gmly\" ,\"status\":0,\"type\":\"GameReportDeviceState\"}";
    c.send(hdl_, msg, websocketpp::frame::opcode::text);
    c.get_alog().write(websocketpp::log::alevel::app, "send msg: " + msg);
    m_timer->cancel();
    m_timer = std::make_shared<boost::asio::deadline_timer>(c.get_io_service(), boost::posix_time::seconds(5));
    m_timer->async_wait(bind(&WsAppConnection::onTimer, this, _1));
}

void WsAppConnection::On_Interrupt(websocketpp::connection_hdl hdl){
    std::string msg = "have a Interrupt!\n";
    LOG_INFO(QString("%1 from %2!").arg(msg.c_str()).arg(m_uri.c_str()));
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
        LOG_ERROR("wsUrl is empty!");
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
        //std::cout << "could not create connection because: " << ec.message() << std::endl;
        LOG_ERROR(QString(" could not create connection because:%1  serverUrl:%2!").arg(ec.message().c_str()).arg(m_uri.c_str()));
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
    LOG_INFO(QString(" WsAppConnection::close from %1!").arg(m_uri.c_str()));
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
    LOG_INFO(QString(" WsAppConnection::closeByAccident from %1!").arg(m_uri.c_str()));
}

void WsAppConnection::terminate()
{
    return;
}




}
