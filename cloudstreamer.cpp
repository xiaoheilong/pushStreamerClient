﻿#include "cloudstreamer.h"
#include "ui_cloudstreamer.h"
#include "globaltools.h"
////////
/// \brief CloudStreamer::CloudStreamer
#pragma comment( lib, "ws2_32" )
#include <QDebug>
#include <WinSock2.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <memory>
#include <iostream>
#include "CloudGame.h"

#include "keyvaluetransformt.h"
#include "recordgameinfo.h"
#include "gamedealexeception.h"
#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QtCore/QTextCodec>
#include <QFileDialog>
#include <functional>
#include <QStringList>
#include <math.h>
#include <bitset>
#include <QTime>
#include <QCryptographicHash>
#include <tlhelp32.h>
using namespace std;
using namespace RecordGameInfoSpace;
using namespace GameDealExeceptionSpace;
//typedef KeyValueTransformtNamespace::KeyValueTransformt  KeyValueTransformt;
//typedef KeyValueTransformtNamespace::DirectionKey   DirectionKey;
//////////////////
/// \brief ExecuteBatScript
/// \param scriptFolder
/// \param scriptName
/// \return
///
///////////////////////some const value
const QString UI_ERROR= "[UI_ERROR]";
const QString UI_INFO= "[UI_INFO]";
const QString UI_WARN= "[UI_WARN]";
extern CloudStreamer * g_This = NULL;
static QString g_gStreamerLogPath = "d://gStreamer.txt";
static QString g_signKey = "CGW1uQ1StIRE0MBVrQg7wcYQjgC2wRlLmAQ1ZofF8S2XKWMv0ZS587lnXZeW9bA1QFo9veTBswC2nAnrMoBUec30mxuWCF0A2h3dOwDcRLjfVrTri73ynYn3gSLBedDSn0S";
static QString g_wsServerKey = "ND857fxx*3";
static int g_reportGameStatusInteral = 8000;  //per ms
const QString g_gstLaunchName = "gst-launch-1.0.exe";

const QString g_autoUpdateScript= "/updater/interface.bat";
const QString g_keyBoardIniTopic="ValueName";
QString g_cloudPathBat="executeFile.bat";
QString g_closeStreamer="/gstreamer/1.0/msvc_x86_64/bin/killStreamer.bat";
const QString g_streamConfigFile = "//streamConfig.ini";
const QString g_cloudStreamConfigFile = "//cloudGameConfig.ini";
//推流和键盘默认配置文件在 streamConfig.ini文件中
//////云游戏默认配置文件在   cloudGameConfig.ini文件中
/// ////////////////
///
BOOL CALLBACK SetTopWindowByProcessId(HWND hwnd,LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd,&lpdwProcessId);
    if(lpdwProcessId==lParam)
    {
        typedef    void    (WINAPI    *PROCSWITCHTOTHISWINDOW)    (HWND,    BOOL);
        PROCSWITCHTOTHISWINDOW    SwitchToThisWindow;
        HMODULE    hUser32    =    GetModuleHandle(L"user32");
        SwitchToThisWindow    =    (    PROCSWITCHTOTHISWINDOW)GetProcAddress(hUser32,    "SwitchToThisWindow");
        SwitchToThisWindow(hwnd , TRUE);
        return FALSE;
    }
    return TRUE;
}

std::string ws2s(const std::wstring& ws)
{
    if (!ws.size()) {
        return "";
    }
    std::string strLocale = setlocale(LC_ALL, "");
    const wchar_t* wchSrc = ws.c_str();
    size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
    char *chDest = new char[nDestSize];
    memset(chDest, 0, nDestSize);
    wcstombs(chDest, wchSrc, nDestSize);
    std::string strResult = chDest;
    delete[]chDest;
    setlocale(LC_ALL, strLocale.c_str());
    return strResult;
}

std::wstring s2ws(const std::string &s)
{
    size_t i;
    std::string curLocale = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, "chs");
    const char* _source = s.c_str();
    size_t _dsize = s.size() + 1;
    wchar_t* _dest = new wchar_t[_dsize];
    wmemset(_dest, 0x0, _dsize);
    mbstowcs_s(&i, _dest, _dsize, _source, _dsize);
    std::wstring result = _dest;
    delete[] _dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}


int isProcessRunning(QString processName)
{
    DWORD pids = 0;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //all processes

    PROCESSENTRY32W entry; //current process
    entry.dwSize = sizeof entry;

    if (!Process32FirstW(snap, &entry)) { //start with the first in snapshot
        return 0;
    }

    do {
        std::wstring exe = entry.szExeFile;
        std::string exe1 =ws2s(exe);
        if (0 == processName.compare(exe1.c_str())) {
           pids=entry.th32ProcessID;
           break;
        }
    } while (Process32NextW(snap, &entry)); //keep going until end of snapshot
    return pids;
}

vector<string> split(const string& str, const string& delim) {
    vector<string> res;
    if ("" == str) return res;
    //先将要切割的字符串从string类型转换为char*类型
    char * strs = new char[str.length() + 1]; //不要忘了
    strcpy(strs, str.c_str());

    char * d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char *p = strtok(strs, d);
    while (p) {
        string s = p; //分割得到的字符串转换为string类型
        res.push_back(s); //存入结果数组
        p = strtok(NULL, d);
    }

    return res;
}
/// p_data 目标字符串  position  类似 10101010 中的位置   flag  1 or 0
void BitSet(unsigned char *p_data, unsigned char position, int flag)
{
    if (p_data) {
        unsigned char tmp = 1;
        if (flag)
        {
            *p_data |= (tmp << (position - 1));
        }
        else
        {
            *p_data &= ~(tmp << (position - 1));
        }
    }
}
string&   replace_all(string&   str,const   string&   old_value,const   string&   new_value)
{
    while(true)   {
        string::size_type   pos(0);
        if(   (pos=str.find(old_value))!=string::npos   )
            str.replace(pos,old_value.length(),new_value);
        else   break;
    }
    return   str;
}

string&   replaceAllDistinct(string&   str,const   string&   old_value,const   string&   new_value)
{
    for(string::size_type   pos(0);   pos!=string::npos;   pos+=new_value.length())   {
        if(   (pos=str.find(old_value,pos))!=string::npos   )
            str.replace(pos,old_value.length(),new_value);
        else   break;
    }
    return   str;
}

void ConvertTheSpecialString(std::string str , BYTE *result , int size1){
    str = replaceAllDistinct(str , " " , "");
    QString value =  QString::fromStdString(str.c_str());
    QStringList array =  value.split(",");
    int size = array.size();
    //std::string result1 ;
    //BYTE *result  = new [16];
    size  = size < size1? size :size1;
    memset(result , 0 , sizeof(BYTE) * size1);
    for (int index = 0; index < size; ++index){
        int  keyValue = array[index].toInt();
        int temp1 = keyValue / 8;
        int temp2 = keyValue % 8;
        BitSet(&(result[temp1]) , temp2 , 1);
        //result1 += result[temp1];
    }
    //return result1;
}

int Power2(int value) {
    if (value < 2) {
        return 0;
    }
    int count = 0;
    do {
        value = value >> 1;
        if (value ==1) {
            ++count;
            break;
        }
        else {
            ++count;
        }
    } while (1);
    return count;
}

void ConvertTheSpecialStringEx(int value , BYTE &result){
//    int temp1 = value / 8;
//    int temp2 = value % 8;
//    for(index = 1 ; index <=temp1; ++index){
//        bitSet(&result , index   , 1);
//    }
    bitset<10> bit(value);
    string str = bit.to_string();
    //BitSet(&result , count   , 1);
    str = str.substr(1 , str.size());
    int size = str.size();
    for (int index = 0; index < size; ++index){
        int value1 = '0' == str.at(index) ? 0 : 1;
        BitSet(&result , size  - index   , value1);
    }
}

//precision 精度 小数点
float GetPercentInRange(float value , float  left , float right){
    if(value < left){
        value = left;
    }
    if(value > right){
        value = right;
    }

    //float difference  = (value  -  left) / (right - left) * 32737;
    double difference  = value  -  left;
    double total = right - left;
    difference = difference / total;
    //float tempValue = 0.00000000 ;
    //float  per = modf(difference ,  &tempValue);
    return difference;
}


void GetIntegerFromFloat(float &value ){
    double integer = 0;
    double decimal = 0.000000;
    decimal = modf(value , &integer);
    value = integer;
    if(0  <=decimal - 0.5){
        value += 1;
    }
}

void JoystickParseMode1(float &X, float &Y, float &Z, float &rX, float &rY, float &rZ, int &slider, int  &dial, int &wheel){
     X = GetPercentInRange(X , -1 , 1) * 32737;
     GetIntegerFromFloat(X);
     Y = GetPercentInRange(Y , -1 , 1) * 32737;
     GetIntegerFromFloat(Y);
     Z = GetPercentInRange(Z , -1 , 1) * 32737;
     GetIntegerFromFloat(Z);
     rX = GetPercentInRange(rX , -1 , 1) * 32737;
     GetIntegerFromFloat(rX);
     rY = GetPercentInRange(rY , -1 , 1) * 32737;
     GetIntegerFromFloat(rY);
     rZ = GetPercentInRange(rZ , -1 , 1) * 32737;
     GetIntegerFromFloat(rZ);
//     slider = GetPercentInRange(slider , -1 , 1) * 32737;
//     GetIntegerFromFloat(slider);
//     dial = GetPercentInRange(dial , -1 , 1) * 32737;
//     GetIntegerFromFloat(dial);
//     wheel = GetPercentInRange(wheel , -1 , 1) * 32737;
//     GetIntegerFromFloat(wheel);
}


void JoystickParseMode(int mode , float &X, float &Y, float &Z, float &rX, float &rY, float &rZ, int &slider, int  &dial, int &wheel){
    switch(mode){
    case 1:{
        JoystickParseMode1(X, Y, Z, rX , rY , rZ , slider , dial , wheel);
    }
        break;
    case 2:{

    }
        break;
    case 3:{

    }
        break;
    default:
        break;
    }
}


int GetWSRequestPrefix(Json::Value & result){
    int ret = -1;
    QTime time;
    time= QTime::currentTime();
    qsrand(time.msec()+time.second()*1000);
    result["noncestr"] = qrand();

    QDateTime dateTime = QDateTime::currentDateTime();
    QString timestamp = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
    int ms = dateTime.time().msec();
    qint64 epochTime = dateTime.toMSecsSinceEpoch();
    //result["timestamp"] = epochTime;
    return  0 ;
}


float ConvertDirect(float x , float y){
    float ret = 0.0;
    return ret ;
}


float ConvertDirection(float z , float rz){
    float ret = 0.0;

    return ret ;
}


QString AssembleWSServer(QString wsUrl ,  QString key , QString deviceNo){
    if(wsUrl.isEmpty() || key.isEmpty() || deviceNo.isEmpty()){
        return "";
    }
    QString resultServer = "ws://";
    //resultServer += "mss.cccsaas.com:4455";
    resultServer += wsUrl;
    //resultServer += "/wss";
    resultServer += "/?deviceNo=";
    resultServer += deviceNo;
    resultServer += "&playSign=";
    QString playSign = deviceNo + g_wsServerKey;
    QDateTime dateTime = QDateTime::currentDateTime();
    QString timestamp = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
    qint64 epochTime = dateTime.toMSecsSinceEpoch();
    QString epochTimeStr = QString("%1").arg(epochTime);
    playSign += epochTimeStr;
    QString signStrMd5 = QCryptographicHash::hash(playSign.toLatin1(),QCryptographicHash::Md5).toHex();

    resultServer += signStrMd5;

    resultServer += "&timeStame=";
    resultServer += epochTimeStr;
    //resultServer += "&loginType=gpuNode";

    return resultServer;
}

////////////
KeyBoardThread::KeyBoardThread(QObject * parent ):QThread(parent) , m_threadFlag(false), m_wsUrl("") , m_loginCommand(""),
    m_wsBoostSocket(NULL), m_keyPingTimer(NULL), m_keyPingThread(NULL), m_reconnectThread(NULL),m_iniParse1(NULL) ,m_iniParse2(NULL),
    m_isUpperKey(false),m_deviceNumberl(""),m_gamePid(0)
{
    connect(this , SIGNAL(AccidentalSignal()) , this , SLOT(AccidentalTermination()),Qt::DirectConnection);
    m_iniParse1 = std::make_shared<DealIniFile>();
    m_iniParse2 = std::make_shared<DealIniFile>();
}

KeyBoardThread::~KeyBoardThread(){
    if(m_keyBoardConfig.get()){
        m_keyBoardConfig.reset();
    }
   CloseWebsocket();
   StopKeyPingTImer();
   StopReconnectThread();
   disconnect(this , SIGNAL(AccidentalSignal()) , this , SLOT(AccidentalTermination()));
   m_iniParse1.reset();
   m_iniParse2.reset();
}

void KeyBoardThread::SetDeviceNumber(QString deviceNum){
    m_deviceNumberl = deviceNum;
}

QString KeyBoardThread::GetDeviceNumber(){
    return m_deviceNumberl;
}

void KeyBoardThread::StartKeyPingTimer(){
    LOG_INFO("StartKeyPingTimer start!");
    if(!m_keyPingTimer){
        m_keyPingTimer =new QTimer(0);
        //m_keyPingTimer->setInterval(800);
        m_keyPingTimer->setInterval(6000);
        m_keyPingThread = new QThread();
        m_keyPingThread->start();
        m_keyPingTimer->moveToThread(m_keyPingThread);
        connect(m_keyPingTimer, SIGNAL(timeout()) , this , SLOT(OnKeyPingTimer()),Qt::DirectConnection);
        connect(m_keyPingThread , SIGNAL(started()) , m_keyPingTimer, SLOT(start()));
        connect(m_keyPingThread , SIGNAL(finished()) , m_keyPingTimer, SLOT(deleteLater()));

        LOG_INFO("StartKeyPingTimer finish init!");
    }else{
        LOG_INFO("StartKeyPingTimer m_keyPingTimer is not nullptr!");
    }
}


void KeyBoardThread::StopKeyPingTImer(){
    LOG_INFO("StopKeyPingTImer start!");
    if(m_keyPingThread){
         disconnect(m_keyPingTimer , SIGNAL(timeout()) , this , SLOT(OnKeyPingTimer()));
         if(!m_keyPingThread->isFinished()){
             if(m_keyPingThread->isRunning()){
                m_keyPingThread->quit();
                m_keyPingThread->wait();
             }
         }
         delete m_keyPingThread;
         m_keyPingThread = NULL;
         LOG_INFO("StopKeyPingTImer finish uninit!");
    }

//    if(m_keyPingTimer && m_keyPingTimer->isActive()){
//        m_keyPingTimer->stop();
//        m_keyPingTimer->deleteLater();
//    }
}







void KeyBoardThread::StartReconnectThread(){
    StopReconnectThread();
    m_reconnectThread = std::make_shared<std::thread>([&](){
        StopAccidental();
        StartAccidental();
    });
    if(m_reconnectThread){
        m_reconnectThread->detach();
    }
}

void KeyBoardThread::StopReconnectThread(){
    if(m_reconnectThread.get()){
        if(m_reconnectThread->joinable()){
            m_reconnectThread->join();
        }
    }
}


void KeyBoardThread::OnKeyPingTimer(){
    LOG_INFO("start keyPing!");
    if(!KeyPing()){
       LOG_INFO("OnKeyPingTimer failure!");
    }else{
       LOG_INFO("OnKeyPingTimer success!");
    }
    if(m_deviceNumberl.isEmpty()){
        LOG_ERROR("OnKeyPingTimer m_deviceNumberl is empty!");
        return;
    }
    //{"mode":2,"port":设备号,"type":"ping","ud":10}
    Json::Value root;

    root["mode"] = 2;
    root["port"] = m_deviceNumberl.toLocal8Bit().data();
    root["type"] = "ping";
    root["ud"] = 10;

    ////////
    Json::FastWriter styled_write;
    std::string rootJsonStr = styled_write.write(root);
    if(m_wsBoostSocket.get()){
        if(0 !=m_wsBoostSocket->Send(rootJsonStr , OpcodeValue::text)){
            LOG_ERROR(QString("KeyBoardThread send msg failure msg=%1").arg(rootJsonStr.c_str()));
        }
    }
}

void KeyBoardThread::StartWebSocket(){
    if(!m_wsUrl.isEmpty()){
        m_wsBoostSocket = std::make_shared<wsBoostConnect>();
        m_wsBoostSocket->init(m_wsUrl.toStdString());
        m_wsBoostSocket->connect();
        m_wsBoostSocket->SetCallback(this);
    }
}

void KeyBoardThread::CloseWebsocket(){
    if(m_wsBoostSocket.get()){
        m_wsBoostSocket->close();
        m_wsBoostSocket.reset();
    }
}

void KeyBoardThread::ConnectedCallback(std::string msg , int error){
    emit RecordSignal(UI_INFO ,QString("ws socket connect successful!").arg(msg.c_str()));
    std::string loginCommand = m_loginCommand.toStdString();
    if(m_wsBoostSocket.get()){
        m_wsBoostSocket->Send(loginCommand);
        emit RecordSignal(UI_INFO , "keyBoard thread start!");
    }else{
        emit RecordSignal(UI_INFO , "keyBoard thread is not started!");
    }
}

void KeyBoardThread::AccidentalTermination(){
    StartReconnectThread();
}

void KeyBoardThread::DisconnectedCallback(std::string msg , int error){
     emit AccidentalSignal();
     emit RecordSignal(UI_ERROR ,QString("ws socket  disconnect  %s!").arg(m_wsUrl.toStdString().c_str()));
}

bool KeyBoardThread::IsLeftKeyboardNumberLetter(int & keyValue){
    if( (65 <= keyValue && 81 >= keyValue) || (0 <= keyValue && keyValue <=9 )){
        return true;
    }
    return false;
}

void KeyBoardThread::DealTheFullKeyboardModel(Json::Value  &root){
    int ud = root["ud"].asInt();
    if (ud == 1)
    {
        bool isInt =  root["key"].isInt();
        bool isString =root["key"].isString();
        int keyTemp = 0;
        if(isInt){
            keyTemp = root["key"].asInt();
        }
        if(isString){
            std::string key = root["key"].asString();
            keyTemp = atoi(key.c_str());
        }
        //////is upper key
        if(!IsLeftKeyboardNumberLetter(keyTemp)){
            if(m_iniParse1.get()){
                QString keyMapping = m_iniParse1->GetValue(g_keyBoardIniTopic , std::to_string(keyTemp).c_str()).toString();
                if(!keyMapping.isEmpty()){//功能按键
                    if(m_iniParse2.get()){
                        QString keyMappingValue = m_iniParse2->GetValue(g_keyBoardIniTopic, keyMapping).toString();
                        if(!keyMappingValue.isEmpty()){
                            int key = keyMappingValue.toInt();
                            if(key > 0){
                                LOG_ERROR(QString("full keyboard down key value:%1").arg(key));
                                KeyDown(key);
                            }
                            if( 20 == key){
                                m_isUpperKey = !m_isUpperKey;
                                LOG_ERROR("upper key is set!");
                            }
                        }else{
                            LOG_ERROR("keyMappingValue is empty!");
                        }
                    }
                }else{
                    LOG_ERROR("keyMapping is empty!");
                }
            }
        }else{//直接处理的键
            if(m_isUpperKey &&( 0 <= keyTemp && 9 >= keyTemp)){
                keyTemp +=32;
            }
            KeyDown(keyTemp);
        }
        LOG_ERROR(QString("full keyboard ud:%1 value=%2").arg(ud).arg(keyTemp));

    }else if (ud == 2)
    {
        bool isInt =  root["key"].isInt();
        bool isString =root["key"].isString();
        int keyTemp = 0;
        if(isInt){
            keyTemp = root["key"].asInt();
        }
        if(isString){
            std::string key = root["key"].asString();
            keyTemp = atoi(key.c_str());
        }
        KeyUp(keyTemp);
        LOG_ERROR(QString("full keyboard ud:%1 value=%2").arg(ud).arg(keyTemp));
    }

}

void KeyBoardThread::DealTheNormalKeyboardModel(Json::Value  &root){
        ////按键
        ///
    int controlType = root["mode"].asInt();
    if( 0 == controlType){//普通键盘按键
        int ud = root["ud"].asInt();
        if(!root.isMember("key")){
            LOG_ERROR("the key message not has member 'key'");
            return;
        }
        if (ud == 1)
        {
            bool isInt =  root["key"].isInt();
            bool isString =root["key"].isString();
            int keyTemp = 0;
            if(isInt){
                keyTemp = root["key"].asInt();
            }
            if(isString){
                std::string key = root["key"].asString();
                keyTemp = atoi(key.c_str());
            }
            KeyDown(keyTemp);
            // emit RecordSignal(UI_INFO ,QString( KEY DOWN: \n").arg(key.c_str()));
        }else
        if (ud == 2)
        {
            bool isInt =  root["key"].isInt();
            bool isString =root["key"].isString();
            int keyTemp = 0;
            if(isInt){
                keyTemp = root["key"].asInt();
            }
            if(isString){
                std::string key = root["key"].asString();
                keyTemp = atoi(key.c_str());
            }
            KeyUp(keyTemp);
            //emit RecordSignal(UI_INFO ,QString("KEY up: \n").arg(key.c_str()));
        }else
        if (ud == 4)
        {
            string keycode = root["key"].asString();
            vector<string> AllStr = split(keycode, "|");
            int x= atoi(AllStr[0].c_str());
            int y = atoi(AllStr[1].c_str());
            int key = atoi(AllStr[2].c_str());
            MouseMove(x, y, key);
        }else
        if (ud == 6)
        {
            string keycode = root["key"].asString();
            vector<string> AllStr = split(keycode, "|");
            int x = atoi(AllStr[0].c_str());
            int y = atoi(AllStr[1].c_str());
            int key = atoi(AllStr[2].c_str());
            MouseDown(x, y, key);
            emit RecordSignal(UI_INFO ,QString("MouseDown up: \n").arg(keycode.c_str()));
        }else
        if (ud == 7)
        {
            string keycode = root["key"].asString();
            vector<string> AllStr = split(keycode, "|");
            int x = atoi(AllStr[0].c_str());
            int y = atoi(AllStr[1].c_str());
            int key = atoi(AllStr[2].c_str());
            MouseUp(x, y, key);
            emit RecordSignal(UI_INFO ,QString("MouseUp up: \n").arg(keycode.c_str()));
        }
    }else if(1 == controlType){/////手柄转换成键盘的按键
        int ud = root["ud"].asInt();
        float x  = 0.0 ;
        float y = 0.0;
        float z = 0.0;
        float rz = 0.0;
        std::string keyStr = "";
        Json::Value keyValue  = root["key"];
        if(keyValue.isMember("x")){
            x = keyValue["x"].asDouble();
        }
        if(keyValue.isMember("y")){
            y = keyValue["y"].asDouble();
        }
        if(keyValue.isMember("z")){
            z = keyValue["z"].asDouble();
        }
        if(keyValue.isMember("rz")){
            rz = keyValue["rz"].asDouble();
        }

        if(keyValue.isMember("key")){
            keyStr = keyValue["key"].asString();
        }
        ///////
        ////方向按键
        if(!m_keyBoardConfig.get()){
            emit RecordSignal(UI_INFO ,QString("m_keyBoardConfig is nullptr  should touch keyboard configure file !\n"));
            return;
        }

        //int xyKey = m_keyBoardConfig->ConvertXYDirection(x, y , ud);
        emit RecordSignal(UI_INFO ,QString("x = %1 y=%2  ud=%3").arg(x ).arg(y).arg(ud));
        ///鼠标
        ///按键
        switch(ud){
        case 1:{ //按下
            if(keyValue.isMember("key")){
                bool isInt =  keyValue["key"].isInt();
                bool isString =keyValue["key"].isString();
                int keyTemp = 0;
                if(isInt){
                    keyTemp = keyValue["key"].asInt();
                }else if(isString){
                    std::string key = keyValue["key"].asString();
                    keyTemp = atoi(key.c_str());
                }
                if(0 != keyTemp){
                    emit RecordSignal(UI_INFO ,QString("keydown normal keyTemp=%1").arg(keyTemp));
                    m_keyBoardConfig->KeyDown_C(keyTemp);
                }
            }
        }
            break;
        case 2:{//抬起
            if(keyValue.isMember("key")){
                bool isInt =  keyValue["key"].isInt();
                bool isString =keyValue["key"].isString();
                int keyTemp = 0;
                if(isInt){
                    keyTemp = keyValue["key"].asInt();
                }else if(isString){
                    std::string key = keyValue["key"].asString();
                    keyTemp = atoi(key.c_str());
                }
                if(0 != keyTemp){
                    emit RecordSignal(UI_INFO ,QString("keyup keyTemp=%1").arg(keyTemp));
                    m_keyBoardConfig->KeyUP_C(keyTemp);
                }
            }
            //                                else if( 0 ==x  && 0 == y){
            //                                        m_keyBoardConfig->DirectionAllUp();
            //                                }

        }
            break;
        case 4:{
            float tempZ = z;
            float tempRz = rz;
            m_keyBoardConfig->ConvertMouse(tempZ , tempRz);
            emit RecordSignal(UI_INFO ,QString("mouse move z=%1 rz=%2 !").arg(z).arg(rz));
            m_keyBoardConfig->MouseMove_C(tempZ, tempRz, 0);//摇杆没有鼠标左中右按键
        }
            break;
        case 6:{

        }
            break;
        case 7:{

        }
            break;
        default:
        {
            break;
        }
        }
    }
}

void KeyBoardThread::DealTheJoystick(Json::Value  &root){
    /// 摇杆
    //bool JoystickCtrl(int X, int Y, int Z, int rX, int rY, int rZ, int slider, int dial, int wheel, BYTE hat, BYTE buttons[16]);
    std::string key = "";
    if(root.isMember("key")){
       // key = root["key"].asString();
        if(! root["key"].empty()){
            Json::Value keyValue  = root["key"];
            float x = 0;
            if(keyValue.isObject()){
                if(keyValue.isMember("x")){
                    x =keyValue["x"].asDouble();
                }

                float y = 0;
                if(keyValue.isMember("y")){
                    y = keyValue["y"].asDouble();
                }
                float z = 0 ;
                if(keyValue.isMember("z")){
                    z = keyValue["z"].asDouble();
                }
                float rX = 0;
                if(keyValue.isMember("rx")){
                    rX = keyValue["rx"].asDouble();
                }
                float rY = 0;
                if(keyValue.isMember("ry")){
                    rY = keyValue["ry"].asDouble();
                }
                float rZ = 0;
                if(keyValue.isMember("rz")){
                    rZ = keyValue["rz"].asDouble();
                }
                int slider = 0;
                if(keyValue.isMember("slider")){
                    slider = keyValue["slider"].asInt();
                }
                int dial = 0;
                if(keyValue.isMember("dial")){
                    dial = keyValue["dial"].asInt();
                }
                int wheel =0;
                if(keyValue.isMember("wheel")){
                    wheel = keyValue["wheel"].asInt();
                }
                int  hat = 0;
                if(keyValue.isMember("hat")){
                    hat = keyValue["hat"].asInt();
                }
                std::string buttons = "";
                if(keyValue.isMember("buttons")){
                    buttons = keyValue["buttons"].asString();
                }

                int mode = 0;
                if(root.isMember("mode")){
                    mode = root["mode"].asInt();
                }
                BYTE *btn = new BYTE[16];
                ConvertTheSpecialString(buttons , btn , 16);
                BYTE hatByte;
                //ConvertTheSpecialStringEx(hat , hatByte);
                ConvertTheSpecialString(buttons , btn , 1);

                JoystickParseMode(mode , x , y, z , rX, rY, rZ , slider , dial, wheel);
                bitset<10> bit(hatByte);
                string hatByteStr = bit.to_string();
                emit RecordSignal(UI_INFO ,QString("x= %1  y=%2  z =%3  rx=%4 ry=%5 rz=%6  slider=%7 dial=%8 wheel=%9  hat=%10 btn=%11\n").arg(x ).arg(y).arg(z).arg(rX).arg(rY).arg(rZ).arg(slider).arg(dial).arg(wheel).arg( hatByteStr.c_str() ).arg( (char*)btn));
                JoystickCtrl(x , y, z , rX, rY, rZ , slider , dial, wheel, hatByte , btn);
                delete []  btn;
                btn = nullptr;
            }
            }


    }
}


bool KeyBoardThread::IsGameWindowActive(){
    bool ret = false;
    HWND hWnd = GetForegroundWindow();
    DWORD processId = 0;
    if(!hWnd){
        return ret;
    }
    GetWindowThreadProcessId(hWnd, &processId);
    LOG_INFO(QString("IsShouldDealKeyMouse processId:%1").arg(processId));

    ///////////get process ID of the gamename
    if( processId == m_gamePid  &&  m_gamePid > 0){
        ret = true;
    }else{
        if(m_gamePid > 0){
            EnumWindows(SetTopWindowByProcessId , m_gamePid);
        }
        LOG_ERROR(QString("game windows is not active , active window pid:%1  gameId:%2").arg(processId).arg(m_gamePid));
    }
    return ret;
}

void KeyBoardThread::SetGamePid(int gamePid){
    if(gamePid){
        m_gamePid = gamePid;
    }
}

int KeyBoardThread::GetGamePid(){
    return m_gamePid;
}

void KeyBoardThread::MessageCallback(std::string message , int error){
    if(!IsGameWindowActive()){
        return ;
    }
    if (message != "")
    {
        emit RecordSignal(UI_INFO , QString("keyboard/mouse msg:%1!").arg(message.c_str()));
        Json::Reader reader;
        Json::Value root;
        root["init"]=123;//prevent the json  Deconstruction crash
        if (reader.parse(message, root))
        {
            std::string type = "";
            if(root.isMember("type")){
                type = root["type"].asString();
            }
            if(0 == type.compare("joystick") ){
                DealTheJoystick(root);

            }else if(0 == type.compare("keycode")){
                ////////全键盘模式
                LOG_ERROR("full keyboard mode");
                DealTheFullKeyboardModel(root);
            }else{
                DealTheNormalKeyboardModel(root);
            }

        }
    }
}

void KeyBoardThread::FailureCallback(std::string msg , int error){
    emit AccidentalSignal();
    emit RecordSignal(UI_ERROR ,QString("ws socket occure failure  %s!").arg(m_wsUrl.toStdString().c_str()));
}

void KeyBoardThread::InterruptCallback(std::string msg , int error){
    emit AccidentalSignal();
    emit RecordSignal(UI_ERROR ,QString("ws socket occure Interrupt  %s!").arg(m_wsUrl.toStdString().c_str()));
}

void KeyBoardThread::run(){
    CloseWebsocket();
    StartWebSocket();
    this->exec();
}



bool KeyBoardThread::start(QString wsUrl , QString loginCommand){
    //////close last start
    if(wsUrl.isEmpty() || loginCommand.isEmpty()){
        LOG_ERROR("KeyBoardThread::start  param is shouldn't be empty!");
        return false;
    }
    m_wsUrl = wsUrl;
    m_loginCommand = loginCommand;
    if(KeyMouseIsValid()){
        KeyMouseClose();
    }
    if(!KeyMouseInit()){
        LOG_ERROR("KeyMouseInit failure!");
         //return false;
    }else{
        LOG_INFO( "KeyMouseInit success!");
    }
    StopKeyPingTImer();
    StartKeyPingTimer();
    /////////
    QString executePath = QCoreApplication::applicationDirPath();
    if(!executePath.isEmpty() && m_iniParse1.get() && m_iniParse2.get()){
        if(0 != m_iniParse1->OpenFile(executePath + "/fullKeyboardValueName.ini") || 0 != m_iniParse1->OpenFile(executePath + "/fullKeyboardNameValue.ini")){
           LOG_ERROR("m_iniParse1 or m_iniParse2 open failure!");
        }
    }else{
        LOG_INFO(QString("executePath: %1  m_iniParse1:%2   m_iniParse2:%3  ").arg(executePath).arg(m_iniParse1.get()?"not null":"null").arg(m_iniParse2.get()?"not null":"null"));
    }
    ///
    QThread::start();
    return true;
}

void  KeyBoardThread::StartAccidental(){
    QThread::start();
}

bool  KeyBoardThread::stop(){
    StopKeyPingTImer();
    if(!KeyMouseClose()){
        LOG_ERROR("KeyMouseClose failure!!");
    }else{
        LOG_ERROR("KeyMouseClose success!!");
    }
    if(isRunning()){
        this->quit();
        this->wait();
    }
    CloseWebsocket();
    LOG_ERROR( " KeyBoardThread quit!");
    return true;
}

void KeyBoardThread::StopAccidental(){
    if(isRunning()){
        this->quit();
        this->wait();
    }
    if(m_wsBoostSocket.get()){
        m_wsBoostSocket->closeByAccident();
        m_wsBoostSocket.reset();
    }else{
        LOG_INFO("StopAccidental m_wsBoostSocket is NULL!");
    }

    LOG_INFO(" KeyBoardThread quit!");
}
///
void KeyBoardThread::SetKeyBoardModel(QString nameKeyTablePath , QString defaultKeyBoardPath , QString gameKeyBoardPath){
    if(nameKeyTablePath.isEmpty() || defaultKeyBoardPath.isEmpty()){
        LOG_ERROR("SetKeyBoardModel keyboardConfigIni is empty!");
        return;
    }
    if(m_keyBoardConfig.get()){
        m_keyBoardConfig.reset();
    }
    m_keyBoardConfig = std::make_shared<KeyValueTransformt>(nameKeyTablePath ,defaultKeyBoardPath , gameKeyBoardPath);
    LOG_INFO( "SetKeyBoardModel keyboardConfigIni succes!");
}


///
bool ExecuteBatScript(QString scriptFolder , QString scriptName){
    if(scriptName.isEmpty() || scriptFolder.isEmpty()){
        return false;
    }
    QProcess p(NULL);
    p.setWorkingDirectory(scriptFolder);
    QString command = scriptName;
    p.start(command);
    if(p.waitForFinished()){
        return true;
    }else{
        QString error  = p.errorString();
        //QMessageBox::information(NULL , "error!" , error);
        LOG_ERROR(QString("ExecuteBatScript execute error: %1").arg(error));
        //}
        return false;
    }

}

void DllLogCallback(std::string logType, std::string logMsg){
    if(g_This && !logType.empty() && !logMsg.empty()){
        QString logtype = logType.c_str();
        QString logmsg = logMsg.c_str();
        LOG_INFO(QString("[%1]:%2").arg(logtype).arg(logmsg));
    }
}




///
CloudStreamer::CloudStreamer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CloudStreamer), m_isInstallDriver(false) , m_isPushStream(false) , m_isOpenKeyBoard(false),m_isWin32Init(false),
    m_keyBoardthreadFlag(false), m_keyBoardThread(nullptr),m_gamePath(""),m_fileSavePath(""),m_mode(DEFAULT_MODE),
    m_sessionId(""), m_deviceNo(""), m_controlUrl("") , m_file(NULL), m_gameStatusTimer(NULL),m_startGameThread(NULL),
    m_stopGameThread(NULL),m_signInThread(NULL),m_changeResolution(NULL),m_gameStatusThread(NULL), m_gstlaunchProtectThead(NULL), m_gstlaunchProtectTheadFlag(false),
    m_systray(NULL),m_gamePid(0),m_StreamConfigIniParse(NULL) , m_CloudGameConfigIniParse(NULL)
{
    ui->setupUi(this);

    ///////////init winsocket 32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        LOG_ERROR("WSAStartup Failed.");
        m_isWin32Init = false;
    }else{
        m_isWin32Init = true;
    }
    //////////////////read the config from streamConfig.ini
    m_StreamConfigIniParse = std::make_shared<DealIniFile>();
    QString executePath = QCoreApplication::applicationDirPath();
    if(executePath.isEmpty()){
        LOG_ERROR("executePath should be empty!");
        return;
    }
    if(0 != m_StreamConfigIniParse->OpenFile(executePath + g_streamConfigFile)){
        LOG_ERROR(QString("the file %1 is open failure!").arg(executePath + g_streamConfigFile));
        return;
    }
    QuitForce(false);
    m_CloudGameConfigIniParse = std::make_shared<DealIniFile>();
    if(0 != m_CloudGameConfigIniParse->OpenFile(executePath + g_cloudStreamConfigFile)){
        LOG_ERROR(QString("the file %1 is open failure!").arg(executePath + g_cloudStreamConfigFile));
        return;
    }
    /////////////
    g_gStreamerLogPath = m_StreamConfigIniParse->GetValue("streamConfig" , "g_gStreamerLogPath").toString();
    g_signKey = m_StreamConfigIniParse->GetValue("streamConfig" , "g_signKey").toString();
    g_wsServerKey = m_StreamConfigIniParse->GetValue("streamConfig" , "g_wsServerKey").toString();
    g_reportGameStatusInteral= m_StreamConfigIniParse->GetValue("streamConfig" , "g_reportGameStatusInteral").toInt();
    //g_reportGameStatusInteral = 30000;//streamConfig.GetValue("streamConfig" , "g_reportGameStatusInteral").toInt();
    LOG_INFO(QString("g_gStreamerLogPath =%1 g_signKey=%2 g_wsServerKey=%3  reportGameTime=%4 ").arg(g_gStreamerLogPath).arg(g_signKey).arg(g_wsServerKey).arg(g_reportGameStatusInteral));
    ////////
    QString streamServerUrl = "https://";
    streamServerUrl += m_StreamConfigIniParse->GetValue("streamConfig" , "videoIp").toString();
    streamServerUrl += ":";
    streamServerUrl += m_StreamConfigIniParse->GetValue("streamConfig" , "videoPort").toString();
    streamServerUrl += "/";
    ///////////////初始值
    ui->lineEdit->setText(streamServerUrl);
    ui->lineEdit_2->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "videoIp").toString());
    ui->lineEdit_3->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "roomId").toString());
    ui->lineEdit_4->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "framerate").toString());
    ui->lineEdit_5->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "bitrate").toString());
    ui->lineEdit_6->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "deadline").toString());
    ui->lineEdit_7->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "cpuused").toString());
    ui->lineEdit_8->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "x").toString());
    ui->lineEdit_9->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "y").toString());
    ui->comboBox->setCurrentIndex(m_StreamConfigIniParse->GetValue("streamConfig" , "mode").toInt());
    ui->comboBox_2->setCurrentIndex(m_StreamConfigIniParse->GetValue("streamConfig" , "capmode").toInt());
    ui->comboBox_3->setCurrentIndex(m_StreamConfigIniParse->GetValue("streamConfig" , "vol").toInt());
    ui->lineEdit_11->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "keyboardIp").toString());
    ui->lineEdit_10->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "keyboardPort").toString());
    ui->textEdit->setText(m_StreamConfigIniParse->GetValue("streamConfig" , "keyboardLoginParams").toString());
    SetUIModel((UI_MODE)(ui->comboBox_4->currentIndex()));
    install_Driver();
    //////////////connect the signal and slot
    m_cloudGameServiceIterator.reset();
    ///
    ///
    g_This = this;
    SetRunLogCallback(DllLogCallback);
    //////////
    //m_file = fopen("c:\\cloudStreamer.txt" , "a+");
//    ui->groupBox_3->setEnabled(false);
//    ui->groupBox_9->setEnabled(false);
//    ui->groupBox_2->setEnabled(false);

//    ui->groupBox_7->setEnabled(false);
//    ui->groupBox->setEnabled(false);
    //////////connect some signal to slot
    connect(this , SIGNAL(ChangeCloudStreamerStatue(QString)) , this , SLOT(on_changeCloudStreamerStatue(QString)));
    connect(this , SIGNAL(InputLog(QString, QString)) , this , SLOT(on_inputLog(QString, QString)));
    ///////
    StartAutoUpdate();
    //InitSystemTray();
    setWindowFlags(Qt::WindowCloseButtonHint);
    //////////从配置文件中读取上一次的gameId
    RecordGameInfo* recordInfos1 = RecordGameInfo::GetInstance();
    m_gameId = recordInfos1->GetGameId();
    ///////////init the params for zero
    ClearFunctionParams();
    //this->showMinimized();
    LOG_INFO(QString("CloudStreamer::CloudStreamer"));
    LastGaspGoalPushStreamer();
//    int id = isProcessRunning("hollow_knight.exe");
//    LOG_ERROR(QString("testFunction:%1").arg(id));
}


CloudStreamer::~CloudStreamer()
{
    uninstall_Driver();
    WSACleanup();
    if(m_cloudGameServiceIterator.get()){
        disconnect(m_cloudGameServiceIterator.get() ,SIGNAL(ParseMessage(QString )), this , SLOT(ParseMessageCallback(QString)));
        m_cloudGameServiceIterator.reset();
    }else{
        LOG_INFO("~CloudStreamer  m_cloudGameServiceIterator is null!");
    }
    if(m_StreamConfigIniParse.get()){
        m_StreamConfigIniParse.reset();
    }else{
        LOG_INFO("~CloudStreamer  m_StreamConfigIniParse is null!");
    }
    if(m_keyBoardThread.get()){
        m_keyBoardThread->stop();
        disconnect(m_keyBoardThread.get() , SIGNAL(RecordSignal(QString , QString)) , this , SLOT(RecordSignalCallBack(QString , QString )));
        m_keyBoardThread.reset();
    }else{
         LOG_INFO("~CloudStreamer  m_keyBoardThread is null!");
    }
    StopReportStatusTimer();
    ///////////disconnect signal-slot
    disconnect(this , SIGNAL(ChangeCloudStreamerStatue(QString)) , this , SLOT(on_changeCloudStreamerStatue(QString)));

    StopWorkThread(m_startGameThread);
    StopWorkThread(m_stopGameThread);
    StopWorkThread(m_signInThread);
    StopWorkThread(m_changeResolution);
    disconnect(this , SIGNAL(InputLog(QString, QString)) , this , SLOT(on_inputLog(QString, QString)));
    CloseAutoUpdate();
    //UInitSystemTray();
    //////////
    RecordGameInfo::ReleaseInstance();
    /////////
    //fclose(m_file);
    LOG_INFO(QString("CloudStreamer::~CloudStreamer"));
    delete ui;
}


void CloudStreamer::ClearFunctionParams(){
    memset(&m_pushStreamerParams , 0 , sizeof(m_pushStreamerParams));
    memset(&m_startGameParams , 0 , sizeof(m_startGameParams));
}

int CloudStreamer::InitSystemTray(){
    //设置提示文字
    return 0 ;
    m_systray = new QSystemTrayIcon(this);
    m_systray->setToolTip("CloudStreamer");

    // 设置托盘图标
    m_systray->setIcon(QIcon(":/imag/image/tray/mb.ico"));


    //托盘菜单项
    QMenu * menu = new QMenu();
    menu->addAction(ui->actionExit);
    m_systray->setContextMenu(menu);

    // 关联托盘事件
    connect(m_systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(OnSystemTrayClicked(QSystemTrayIcon::ActivationReason)));

    //显示托盘
    m_systray->show();

    //托盘菜单响应
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(OnExit()));
    return 0;
}


int CloudStreamer::UInitSystemTray(){
    return 0;
    if(m_systray){
        delete m_systray;
        m_systray = NULL;
    }
    return 0;
}

void  CloudStreamer::ProtectGstLaunch(){
    m_gstlaunchProtectTheadFlag = true;
    //////////wait the gst-launch-1.0 is start success
    QString appPath = QCoreApplication::applicationDirPath();
    QString gstExePath = g_gstLaunchName;
    while(0 < isProcessRunning(gstExePath.toLocal8Bit().data()) && m_gstlaunchProtectTheadFlag){
        Sleep(1000);
    }
    LOG_INFO("start enter the loops which check gst-launch-1.0's status!\n");
    ///////////
    while(m_gstlaunchProtectTheadFlag){
        ///////the node is startting game
        RecordGameInfo* recordInfos = RecordGameInfo::GetInstance();
        int gameStatus = recordInfos->GetGameStatus();
        if( 0 == gameStatus){
            break;
        }
        /////check the status of gst-launch-1.0
        bool result = false;
        result = GameIsAreadlyRunning(m_gameId);
        if(result){
            if(m_keyBoardThread.get()){
                m_keyBoardThread->SetGamePid(m_gamePid);
            }
            result  = isProcessRunning(gstExePath.toLocal8Bit().data()) > 0 ? true:false;
            if(!result){
                LOG_INFO("gst-launch-1.0 is quit!\n");
                if(!appPath.isEmpty()){
                    //std::unique_lock<std::mutex> lock(m_gstLaunchMutex);
                    if(!m_pushStreamerParams.m_serverUrl.empty()){
                            //m_pushStreamerFunc();
                        QString logStr =QString("ProtectGstLaunch  params ---->");
                        LOG_INFO(QString("%1:%2").arg(logStr).arg(m_pushStreamerParams.LogStr().c_str()));
                        PushStreamerAction(m_pushStreamerParams.m_serverUrl.c_str(), \
                                           m_pushStreamerParams.m_domain.c_str(), \
                                           m_pushStreamerParams.m_roomId.c_str() ,\
                                           m_pushStreamerParams.m_framerate.c_str(),\
                                           m_pushStreamerParams.m_bitrate.c_str() ,\
                                           m_pushStreamerParams.m_deadline.c_str(),\
                                           m_pushStreamerParams.m_cpuused.c_str(),\
                                           m_pushStreamerParams.m_x.c_str(),\
                                           m_pushStreamerParams.m_y.c_str(), \
                                           m_pushStreamerParams.m_mode.c_str(),\
                                           m_pushStreamerParams.m_capmode.c_str(), \
                                           m_pushStreamerParams.m_vol.c_str());

                    }
                }else{
                    LOG_ERROR("gst-launch-1.0  appPath is empty!\n");
                }

            }else{
                LOG_INFO( "gst-launch-1.0 is running!\n");
            }
        }else{
            //////////////check the status of game
//            result = GameIsAreadlyRunning(m_gameId);
//            if(!result){
//                //m_startGameFunc(0);
//                StartGameAction(m_startGameParams.m_gameId.c_str() , m_startGameParams.m_startGameParams.c_str() , m_startGameParams.m_data.c_str() , 0);
//            }
        }
        Sleep(8000);
    }


    LOG_INFO("quit the loops which check gst-launch-1.0's status!\n");
}

void  CloudStreamer::StartProtectGstLaunch(){
    m_gstlaunchProtectThead = std::make_shared<std::thread>(&CloudStreamer::ProtectGstLaunch , this);
    if(m_gstlaunchProtectThead.get()){
        m_gstlaunchProtectThead->detach();
    }else{
        LOG_ERROR("m_gstlaunchProtectThead init failure!\n");
    }
}

void  CloudStreamer::StopProtectGstLaunch(){
    if(m_gstlaunchProtectThead.get()){
        m_gstlaunchProtectTheadFlag = false;
        if(m_gstlaunchProtectThead->joinable()){
            m_gstlaunchProtectThead->join();
            m_gstlaunchProtectThead.reset();
        }else{
            LOG_INFO("StopProtectGstLaunch  m_gstlaunchProtectThead couldn't joinable!");
        }
    }
}

void CloudStreamer::RecoveryGame(){
    RecordGameInfo* recordInfos = RecordGameInfo::GetInstance();
    int gameStatus = recordInfos->GetGameStatus();
    QString rootJsonStr = recordInfos->GetGameInfo();
    if(gameStatus){
        ///////game is not close normally , then recovery the normal show
        ///if the game's exe is running then show the gameWindow top modal
        ///if the game's exe is doesn't running , then restart the game's exe
        StopWorkThread(m_startGameThread);
        m_startGameThread = std::make_shared<std::thread>(&CloudStreamer::StartGameCallback, this , rootJsonStr , NORMAL_MODEL);
        if(m_startGameThread.get()){
            m_startGameThread->detach();
        }else{
             LOG_INFO("RecoveryGame  m_startGameThread  is null!");
        }
    }
}

void    CloudStreamer::StopWorkThread(std::shared_ptr<std::thread> thread){
    if(thread.get()){
        if(thread->joinable()){
            thread->join();
        }else{
            LOG_INFO("StopWorkThread  thread  is couldn't joinable!");
        }
        thread.reset();
        thread = NULL;
    }
}


void CloudStreamer::QuitForce(bool value){
    QString executePath = QCoreApplication::applicationDirPath();
    if(executePath.isEmpty()){
        LOG_ERROR("executePath should be empty!");
        return;
    }
    if(!m_StreamConfigIniParse.get()){
        m_StreamConfigIniParse = std::make_shared<DealIniFile>();
        int ret  = m_StreamConfigIniParse->OpenFile(executePath + g_streamConfigFile);
        if(0 != ret){
            LOG_ERROR(QString("QuitForce streamConfig open failure the file path=%1 !").arg(executePath + g_streamConfigFile));
            return;
        }
    }
    m_StreamConfigIniParse->SetValue("streamConfig" , "forceQuit", value ?"1":"0");
}

void CloudStreamer::KillGameByName(QString gameName){
    QString params = "stop ";
    params += " \"";
    params += gameName;
    params += "\"";
    QString appPath = QCoreApplication::applicationDirPath();
    QString str = appPath + "/" +  g_cloudPathBat;
    StartGame(str.toLocal8Bit().data()  ,params.toLocal8Bit().data(),SW_HIDE);
}

void CloudStreamer::closeEvent ( QCloseEvent * event ){
    ////////////////tray
    this->hide();
    event->ignore();
}

void  CloudStreamer::changeEvent(QEvent * event){
    if(event->type()!=QEvent::WindowStateChange) {
        return;
    }
//    Qt::WindowStates states = this->windowState();
//    if(  states ==Qt::WindowMinimized  || states ==Qt::WindowNoState){
//        this->hide();
//        m_systray->show();
//    }
}

int CloudStreamer::OnSystemTrayClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger )
    {
        // 显示主窗口
        this->showNormal();
    }else if(reason == QSystemTrayIcon::DoubleClick){
        this->hide();
    }
    return 0;
}


int CloudStreamer::OnExit(){
//    if(m_systray){
//        m_systray->deleteLater();
//    }else{
//        LOG_INFO("OnExit  m_systray is null!");
//    }
    /////////////
    QuitForce(true);
    if(m_gameId.isEmpty()){
         LOG_INFO("gameId or  startGameParams is empty!\n");
    }
    QString gamePath = GetValueByGameID(m_gameId , "gameExeName");;//GetGameStopByID(m_gameId);
    if(!gamePath.isEmpty()){
        KillGameByName(gamePath);
        LOG_INFO("game stop success!\n");
    }
    if(ui->pushButton_3){
        ui->pushButton_3->clicked();
    }else{
        LOG_ERROR("OnExit ui->pushButton_3 is NULL!");
    }
    //QApplication::exit(0);
    return 0;
}


void CloudStreamer::BindServiceIterator(std::shared_ptr<CloudGameServiceIterator>  iterator){
    if(m_cloudGameServiceIterator){
          disconnect(m_cloudGameServiceIterator.get() ,SIGNAL(ParseMessage(QString )), this , SLOT(ParseMessageCallback(QString)));
    }
   m_cloudGameServiceIterator.reset();
   m_cloudGameServiceIterator = iterator;
   connect(m_cloudGameServiceIterator.get() ,SIGNAL(ParseMessage(QString )), this , SLOT(ParseMessageCallback(QString)));
}
///////
std::shared_ptr<CloudStreamer> CloudStreamer::GetPtr(){
    return shared_from_this();
}


void CloudStreamer::SetUIModel(UI_MODE model){
    m_mode = model;
    switch(model){
        case ONLY_PUSH_STREAMER:{
            ui->groupBox_3->setEnabled(false);
            ui->groupBox_9->setEnabled(false);
            ui->groupBox_2->setEnabled(false);

            ui->groupBox_7->setEnabled(true);
            ui->groupBox->setEnabled(true);
        }
            break;
        case ONLY_KEY_BOARD:{
            ui->groupBox_3->setEnabled(false);
            ui->groupBox_9->setEnabled(false);
            ui->groupBox->setEnabled(false);

            ui->groupBox_7->setEnabled(true);
            ui->groupBox_2->setEnabled(true);
            //ui->groupBox_7->hide();
        }
        break;
        default:{
            ui->groupBox_3->setEnabled(true);
            ui->groupBox_9->setEnabled(true);
            ui->groupBox->setEnabled(true);

            ui->groupBox_7->setEnabled(true);
            ui->groupBox_2->setEnabled(true);
            break;

        }
    }
}
////////初始化环境
void CloudStreamer::install_Driver()
{
    return;
    QString runtimePath = QCoreApplication::applicationDirPath();
    if(!runtimePath.isEmpty()){
        //QString  installAllBat = runtimePath +  "./Drivers/InstallAll.bat";
        if(!ExecuteBatScript(runtimePath , "Drivers/InstallAll.bat")){
            LOG_ERROR("InstallAll.bat Failed.\n");
            return;
        }
    }else{
        LOG_ERROR("QDir::currentPath is empty!.\n");
    }

}
////////卸载环境
void CloudStreamer::uninstall_Driver()
{
    return;
    QString runtimePath = QCoreApplication::applicationDirPath();
    if(!runtimePath.isEmpty()){
        if(!ExecuteBatScript(runtimePath , "Drivers/UninstallAll.bat")){
            return;
        }

    }else{
    }
}


 void  CloudStreamer::addLogToEdit(QString flagStr,QString logStr){
      emit InputLog(flagStr , logStr);
 }


 void CloudStreamer::on_inputLog(QString flagStr,QString logStr){
     if(!logStr.isEmpty()){
         SYSTEMTIME sys;
         GetLocalTime( &sys );
         QString str = flagStr;
         str.sprintf("%4d/%2d/%2d %2d:%2d:%2d.%3d", sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
         str +=":";
         str += logStr;
         str += "\n";
         LOG_INFO(QString("%1").arg(str));
         //ui->textEdit_2->append(str);

//         if(m_file){
//             fwrite(str.toLocal8Bit().data(), str.size(), 1 , m_file);
//         }
     }
 }

 ////////////////////test
 // QMessageBox *msgbPtr;//在子线程内创建的messageBox会有问题，所以只能在主线程创建并传递过过来
 // void initMessageBox()
 // {
 //     return;
 //     msgbPtr = new QMessageBox();
 //     msgbPtr->setWindowFlag(Qt::WindowStaysOnTopHint);
 //     msgbPtr->setIcon(QMessageBox::Critical);
 //     msgbPtr->setWindowTitle("软件崩溃");
 //     msgbPtr->setText("未知原因崩溃，程序将在5s后重启");

 //     QPushButton *rebootButton;
 //     QPushButton *rejectButton ;

 //     rebootButton = msgbPtr->addButton("重启", QMessageBox::AcceptRole);
 //     rejectButton = msgbPtr->addButton("退出", QMessageBox::RejectRole);

 //     QObject::connect(msgbPtr, &QMessageBox::buttonClicked, [&](QAbstractButton *button){


 //         if(msgbPtr->clickedButton() == (QAbstractButton*)rebootButton)
 //         {
 //             qDebug() << "start application:/n";//重启
 // //            qApp->quit();
 //         }
 //         //QProcess::startDetached(qApp->applicationFilePath(), QStringList());
 //         msgbPtr->close();
 //     });
 // }
////////开启推流
void CloudStreamer::on_pushButton_2_clicked()
{

    //////
    SetGStreamerLogPath(g_gStreamerLogPath.toLocal8Bit().data());
    QString roomName = ui->lineEdit_3->text();
    QString pushServer = ui->lineEdit->text();
    QString pushDomain = ui->lineEdit_2->text();
    QString framerate = ui->lineEdit_4->text();
    QString bitrate = ui->lineEdit_5->text();
    //int deadline, int cpuused, int x, int y, int mode, int capmode, int vol
    QString deadline = ui->lineEdit_6->text();
    QString cpuused = ui->lineEdit_7->text();
    QString screenWidth= ui->lineEdit_8->text();
    QString screenHeight= ui->lineEdit_9->text();
    int mode = ui->comboBox->currentIndex();
    int capmode = ui->comboBox_2->currentIndex();
    int vol = ui->comboBox_3->currentIndex();
    std::string roomNameStr = roomName.toStdString();
    std::string pushServerStr = pushServer.toStdString();
    std::string pushDomainStr = pushDomain.toStdString();
    bool ret = false;
    char * str1 = const_cast<char*>(roomNameStr.c_str());
    char* str2 = const_cast<char*> (pushServerStr.c_str());
    char * str3 = const_cast<char*>(pushDomainStr.c_str());
    int framerateTemp = framerate.toInt();
    int bitrateTemp = bitrate.toInt();
    int deadlineTemp = deadline.toInt();
    int cpuuseTemp =  cpuused.toInt();
    int screenWidthTemp =  screenWidth.toInt();
    int screenHeightTemp = screenHeight.toInt();
    // SetUIModel((UI_MODE)(ui->comboBox_4->currentIndex()));
    if( UI_MODE::DEFAULT_MODE == m_mode || UI_MODE::ONLY_PUSH_STREAMER == m_mode){
        ret = closepush(str1 , str2);
        if(!ret){
             //QMessageBox::information(this , "information!" ,"close stream failure!\n");
             //return;
        }
        ret =push(str1,str2, str3,framerateTemp , bitrateTemp,deadlineTemp , cpuuseTemp,
            screenWidthTemp, screenHeightTemp, mode, capmode, vol);
        if(!ret){
             //QMessageBox::information(this , "information!" ,"push stream failure!\n");
             LOG_ERROR("push stream failure!\n");
             return;
        }
    }
    /////////
    if(UI_MODE::DEFAULT_MODE == m_mode || UI_MODE::ONLY_KEY_BOARD == m_mode){
        if(m_keyBoardThread.get()){
            m_keyBoardThread->stop();
            disconnect(m_keyBoardThread.get() , SIGNAL(RecordSignal(QString , QString)) , this , SLOT(RecordSignalCallBack(QString , QString )));
            m_keyBoardThread.reset();
        }
        //std::weak_ptr<CloudStreamer> weakPtr = std::shared_ptr<CloudStreamer>(this);
        m_keyBoardThread = std::make_shared<KeyBoardThread>();
        connect( m_keyBoardThread.get() , SIGNAL(RecordSignal(QString , QString)) , this , SLOT(RecordSignalCallBack(QString , QString )));
        QString wsUrl = "wss://";
        wsUrl += ui->lineEdit_11->text();
        wsUrl += ":";
        wsUrl +=  ui->lineEdit_10->text();
        wsUrl += "/wss";

        QString loginCommand = ui->textEdit->toPlainText();
        m_keyBoardThread->start(wsUrl ,loginCommand);
    }
    ui->label_16->setText("start!");
    //QMessageBox::information(this , "information!" ,"push stream success!\n");
    //QMessageBox::information(nullptr , "information!" ,"start success!\n");
}
////////关闭推流
void CloudStreamer::on_pushButton_3_clicked()
{
    LOG_INFO("enter on_pushButton_3_clicked!");
    if(ui->lineEdit_3  &&  ui->label_16){
        QString roomName = ui->lineEdit_3->text();
        QString pushServer = ui->lineEdit->text();
        std::string roomNameStr = roomName.toStdString();
        std::string pushServerStr = pushServer.toStdString();
        if(UI_MODE::DEFAULT_MODE == m_mode || UI_MODE::ONLY_PUSH_STREAMER == m_mode){
            closepush(const_cast<char*>(roomNameStr.c_str()),const_cast<char*> (pushServerStr.c_str()));
        }
        ///////
        if(UI_MODE::DEFAULT_MODE == m_mode || UI_MODE::ONLY_KEY_BOARD == m_mode){
            if(m_keyBoardThread.get()){
                m_keyBoardThread->stop();
                disconnect(m_keyBoardThread.get() , SIGNAL(RecordSignal(QString , QString)) , this , SLOT(RecordSignalCallBack(QString , QString )));
                m_keyBoardThread.reset();
            }
        }
        ui->label_16->setText("stop!");
    }else{
        LOG_ERROR("ui->lineEdit_3  or  ui->label_16 is null!");
    }
}
////////连接wsServer
void CloudStreamer::on_pushButton_4_clicked()
{
    //////
//    QString  wsUrl = ui->lineEdit_16->text();
//    if(wsUrl.isEmpty()){
//        //QMessageBox::information(this , "error!" ,"wsUrl shouldn't be empty!\n");
//        addLogToEdit(UI_ERROR , "wsUrl shouldn't be empty!\n");
//        return ;
//    }
    char *testValue = NULL;
    int testNumber = strlen(testValue);
    std::cout<<*testValue<<std::endl;

}
////////关闭键盘
//void CloudStreamer::on_pushButton_5_clicked()
//{
////    if(m_keyBoardThread){
////     m_keyBoardThread->quit();
////    }
//}
//启动游戏
void CloudStreamer::on_pushButton_6_clicked()
{
    std::string  gamePaths   =  ui->lineEdit_14->text().toStdString();
    std::string  gameParames = ui->lineEdit_12->text().toStdString();
    if(gamePaths.empty()){
        //QMessageBox::information(this , "error!" ,"gamePaths shouldn't be empty!\n");
        LOG_ERROR("gamePaths shouldn't be empty!\n");
        return;
    }
    char *gameStr =  const_cast<char*>(gamePaths.c_str());
    char *gameStr1  = const_cast<char*>(gameParames.c_str());
    if(gameStr && gameStr1){
        if(!gameParames.empty()){
            StartGame(gameStr,  gameStr1);
        }else{
            char defaultStr[20];
            memset(defaultStr , 0 , sizeof(defaultStr));
            int size = strlen("123");
            memcpy(defaultStr , "123" , size);
            defaultStr[size] = '\0';
            StartGame(gameStr, defaultStr );
        }
    }
}
////下载文件
void CloudStreamer::on_pushButton_7_clicked()
{
    std::string  fileUrls = ui->lineEdit_13->text().toStdString();
    std::string  savePaths = ui->lineEdit_15->text().toStdString();
    if(fileUrls.empty() || savePaths.empty()){
       // QMessageBox::information(this , "error!" ,"fileUrls or savePaths shouldn't be empty!\n");
        LOG_ERROR("fileUrls or savePaths shouldn't be empty!\n");
        return ;
    }

    char *gameStr =  const_cast<char*>(fileUrls.c_str());
    char *gameStr1  =  const_cast<char*>(savePaths.c_str());
    if(gameStr && gameStr1){
        bool re = wfile(gameStr , gameStr1);
        if(!re){
            //QMessageBox::information(this , "error!" ,"wfile is failure!\n");
            LOG_ERROR("wfile is failure!\n");
            return;
        }else{
             //QMessageBox::information(this , "error!" ,"wfile is success!\n");
            LOG_ERROR("wfile is success!\n");
        }
    }
}

//////打开游戏选择路径
void CloudStreamer::on_pushButton_clicked()
{
    QString gamePath = QFileDialog::getOpenFileName(this, "选择目录", "./", "*.exe");
    if(!gamePath.isEmpty()){
        m_gamePath = gamePath;
    }
    ui->lineEdit_14->setText(m_gamePath);
}

//////////保存下载文件
void CloudStreamer::on_pushButton_8_clicked()
{
    QString fileSavePath = QFileDialog::getSaveFileName(this, "选择目录", "./", "*");
    if(!fileSavePath.isEmpty()){
        m_fileSavePath = fileSavePath;
    }
    ui->lineEdit_15->setText(m_fileSavePath);
}


void CloudStreamer::ConnectCallback(QString url, QString data){

}

void CloudStreamer::DisconnectCallback(QString url, QString data){

}

void CloudStreamer::GameStatusCallback(){
    QString gameName = GetValueByGameID(m_gameId , "gameExeName");
    RecordGameInfo *recordInfos1 = RecordGameInfo::GetInstance();
    if(isUpdate() ){
        LOG_INFO( "pushStreamer is updating!");
        recordInfos1->RecordInfo(1);
    }
    QString  testStr = WSServiceTransferSignStringEx(m_deviceNo , m_sessionId , m_gameId ,gameName);
    emit m_cloudGameServiceIterator->Send(testStr);
    LOG_INFO(QString("send GameReportDeviceState status: %1").arg(testStr));
}


bool CloudStreamer::isUpdate(){
    //RecordGameInfo recordInfos1;
    return  false;
    QString executePath = QCoreApplication::applicationDirPath();
    if(executePath.isEmpty()){
        LOG_ERROR( "executePath should be empty!");
    }
    if(!m_StreamConfigIniParse.get()){
        m_StreamConfigIniParse = std::make_shared<DealIniFile>();
        int ret =m_StreamConfigIniParse->OpenFile(executePath + g_streamConfigFile);
        if(0 != ret){
            LOG_ERROR(QString("open %1  ini file is failure!").arg(executePath + g_streamConfigFile));
            return false;
        }
    }

    QString str = m_StreamConfigIniParse->GetValue("streamConfig" , "isUpdate").toString();
    if(!str.isEmpty()){
        if(1 == str.toInt()){
            return true;
        }
    }
    return false;
}

void CloudStreamer::LastGaspGoalPushStreamer(){
//    QString appPath = QCoreApplication::applicationDirPath();
//    QString str11 = appPath + g_closeStreamer;
//    QProcess process;
//    process.setProgram("cmd");
//    QStringList argument;
//    argument<<"/c"<< "taskkill  /IM gst-launch-1.0.exe  /F" ;//"start /min"<<str11.toLocal8Bit().data() ;
//    process.setArguments(argument);
//    process.start();
//    //process.waitForStarted(); //等待程序启动
//    process.waitForFinished();//等待程序关闭
    QProcess p;
    QString c = "taskkill /im gst-launch-1.0.exe /f";
    p.execute(c);
    p.close();
}

void CloudStreamer::StartGameCallback(QString data , StartGameModel model){
    LOG_INFO("enter StartGameCallback  !");
    LOG_INFO(data);
    //std::unique_lock<std::mutex> lock(m_gameMutex);
    ClearFunctionParams();
    LastGaspGoalPushStreamer();
    try{
        if(!data.isEmpty()){
            Json::Reader reader;
            Json::Value root;
            root["init"] = 123;
            if (reader.parse(data.toStdString(), root)){
                QString gameId = root["gameId"].asCString();
                if(isUpdate()){
                    LOG_INFO("pushStreamer is updating!");
                    RecordGameInfo *recordInfos1 = RecordGameInfo::GetInstance();
                    recordInfos1->RecordInfo(1);
                    QString gameName = GetValueByGameID(gameId , "gameExeName");
                    QString  testStr = WSServiceTransferSignStringEx(m_deviceNo , m_sessionId , gameId ,gameName);
                    emit m_cloudGameServiceIterator->Send(testStr);
                    throw  GameDealExeception("pushStreamer is updating!" , -1);
                    //return;
                }else{

                     ForeachKillExe(gameId);
/*
                    RecordGameInfo* recordInfos1 = RecordGameInfo::GetInstance();
                    m_gameId = recordInfos1->GetGameId();
                    if(0 != m_gameId.compare(gameId) && !m_gameId.isEmpty()){
                        QString gameName1 = GetValueByGameID(m_gameId , "gameExeName");
                        LOG_INFO(QString("kill the laste game:%1").arg(gameName1));
                        KillGameByName(gameName1);
                    }else{

                    }
*/
                }
                m_gameId = gameId;
                //QString startGameParams = root["startGameParams"].asCString();
                QString roomId = root["roomId"].asCString();

                QString playerUrl = root["playerUrl"].asCString();//serverUrl
                QString serverUrl = root["serverUrl"].asCString();
                QString controlUrl = root["controlUrl"].asCString();
                QString port = root["port"].asCString();
                //           QString videoIp = root["videoIp"].asCString();
                //           QString peerId = root["peerId"].asCString();
                //           QString videoPort = root["videoPort"].asCString();
                //           QString keyboardIp = root["keyboardIp"].asCString();
                //           QString keyboardPort = root["keyboardPort"].asCString();
                QString framerate = root["framerate"].asCString();
                QString bitrate = root["bitrate"].asCString();
                QString deadline = root["deadline"].asCString();
                QString cpuused = root["cpuused"].asCString();
                QString x = root["x"].asCString();
                QString y = root["y"].asCString();
                QString mode = root["mode"].asCString();
                QString capmode = root["capmode"].asCString();
                QString vol = root["vol"].asCString();

                QString control = root["control"].asCString();
                QString fileMiniTime = root["fileMiniTime"].asCString();
                QString isQueuing = root["isQueuing"].asCString();
                int  force = 0 ;
                if(root.isMember("force")){
                    force = root["force"].asInt();
                }
                ///////////////////
                /// \brief keyboardLoginParams
                if(root["keyboardLoginParams"].isObject()){
                    Json::Value loginRoot =  root["keyboardLoginParams"];
                    loginRoot["init"]=123;//just for promise the Json::Value has a init value
                    loginRoot["port"] = m_deviceNo.toLocal8Bit().data();
                    root["keyboardLoginParams"] = loginRoot;
                }

                Json::FastWriter styled_write;
                std::string tempStr = styled_write.write(root["keyboardLoginParams"]);
                QString keyboardLoginParams = tempStr.c_str();
                /////////////

                QString startGameParams = root["startGameParams"].asCString();
                //////////
                ///////////////
                int ret =  -1;
                if(serverUrl.isEmpty()|| controlUrl.isEmpty() || port.isEmpty() || roomId.isEmpty()|| gameId.isEmpty() || keyboardLoginParams.isEmpty()){
                    LOG_ERROR("StartGameCallback json param format failure!");
                    throw GameDealExeception("StartGameCallback json param format failure!" , -2);
                    //return ;
                }
                /////////

                QString domain = serverUrl;
                QString pushUrl = "https://";
                serverUrl = pushUrl + serverUrl;
                serverUrl +=  ":";
                serverUrl += "4443";
                serverUrl += "/";
                ////////loginParam port replace to deviceNo
                StartGameAction(gameId , startGameParams , data, force);
                /////////////

                /////////////////////
                PushStreamerAction(serverUrl,domain,roomId ,framerate,bitrate ,deadline,cpuused,x,y,mode,capmode, vol);
                /////////////

                /////////
                if(NORMAL_MODEL == model){
                    StartKeyboardAction(gameId , controlUrl , keyboardLoginParams);
                }
                LOG_INFO("keyBoard connection finish!\n");
                //////////////////set the Delay bubble to report StartGame result
            }
        }
    }catch(GameDealExeception  e){
        ////////////exeception deal
        MessageFeedBack("StartGame" , e.GetErrorCode().c_str() , e.what());

    }
}


int  CloudStreamer::PushStreamerAction(QString serverUrl , QString domain , QString roomId , QString framerate, QString bitrate ,QString deadline , QString cpuused ,QString x , QString y,QString mode, QString capmode , QString vol)
{
        m_pushStreamerParams.Set(serverUrl,domain,roomId ,framerate,bitrate ,deadline,cpuused,x,y,mode,capmode, vol);
//                m_pushStreamerFunc = std::bind([serverUrl,domain,roomId ,framerate,bitrate ,deadline,cpuused,x,y,mode,
//                                               capmode, vol, this]{
        int ret = closepush(roomId.toLocal8Bit().data() , serverUrl.toLocal8Bit().data());
        if(!ret){
            LOG_ERROR("close stream failure!\n");
            return -3;
        }
        QString logStr =QString("pushStream  params ---->");
        LOG_INFO(QString("%1:%2").arg(logStr).arg(m_pushStreamerParams.LogStr().c_str()));
        ret = push(roomId.toLocal8Bit().data(),serverUrl.toLocal8Bit().data(), domain.toLocal8Bit().data(),framerate.toInt() ,
                   bitrate.toInt(),deadline.toInt() , cpuused.toInt(),x.toInt(),
                   y.toInt(), mode.toInt(), capmode.toInt(), vol.toInt());
        if(!ret){
            LOG_ERROR("push stream failure!\n");
            return -4;
        }
        LOG_INFO("push stream success!\n");
    //});
    //m_pushStreamerFunc();
    return 0;
}

int  CloudStreamer::StartGameAction(QString gameId , QString startGameParams , QString data, int force){
    m_startGameParams.SetValue(gameId , startGameParams , data ,force);
   // m_startGameFunc = std::bind([gameId , startGameParams,this ,  data](int forceValue){
        QString startGameParamsEx = startGameParams;
        if(gameId.isEmpty()  || startGameParamsEx.isEmpty()){
            LOG_INFO("gameId or  startGameParamsEx is empty!\n");
        }else{
            if(0 == startGameParamsEx.compare("null")){
                LOG_INFO(QString("%1 need restart gst-launch-1.0.exe").arg(gameId));
                startGameParamsEx = "";
                ActiveReportGameStatus();
            }
        }
        bool gameIsRunning = false;
        gameIsRunning = GameIsAreadlyRunning(gameId);
        if(gameIsRunning){
            if(/*forceValue*/force > 0 ){
                LOG_INFO(QString("%1 been force kill!").arg(gameId));
                KillAreadlyRunningGame(gameId);
                StartGameByGameId(gameId , startGameParamsEx );
            }
        }else{
            LOG_INFO(QString("%1 is not running ,should been restart now!").arg(gameId));
            StartGameByGameId(gameId , startGameParamsEx);
        }
        do{
            gameIsRunning = GameIsAreadlyRunning(gameId);
            if(!gameIsRunning){
                Sleep(2000);
            }else{
                break;
            }
        }while(true);
        RecordGameInfo *recordInfos1 = RecordGameInfo::GetInstance();
        recordInfos1->RecordInfo(data, gameId, 1);
    //}, _1);
//                if(NORMAL_MODEL == model){
//                    m_startGameFunc(force);
//                }
        return 0;
}


int  CloudStreamer::StartKeyboardAction(QString gameId , QString controlUrl , QString keyboardLoginParams){
    if(m_keyBoardThread.get()){
        m_keyBoardThread->stop();
        disconnect(m_keyBoardThread.get() , SIGNAL(RecordSignal(QString , QString)) , this , SLOT(RecordSignalCallBack(QString , QString )));
        m_keyBoardThread.reset();
    }
    m_keyBoardThread = std::make_shared<KeyBoardThread>();
    connect(m_keyBoardThread.get() , SIGNAL(RecordSignal(QString , QString)) , this , SLOT(RecordSignalCallBack(QString , QString )));
    QString appPath = QCoreApplication::applicationDirPath();
    QString str11 = appPath + "/";
    QString nameKeyTablePath = GetValueByGameID(gameId , "keyBoardNameValueMappingTable");
    nameKeyTablePath = str11 + nameKeyTablePath;
    QString defaultKeyBoardPath  = GetValueByGameID(gameId , "keyBoardDefaultConfig");
    defaultKeyBoardPath = str11 + defaultKeyBoardPath;
    QString gameKeyBoardPath = GetValueByGameID(gameId , "keyBoardGameConfig");
    m_keyBoardThread->SetKeyBoardModel(nameKeyTablePath , defaultKeyBoardPath , gameKeyBoardPath);
    //           QString wsUrl = "ws://";
    //           controlUrl = wsUrl + controlUrl;
    //           controlUrl += "/";
    m_controlUrl = controlUrl;
    QString wsUrl  = AssembleWSServer(controlUrl , g_wsServerKey , m_deviceNo);
    //ui->textEdit->setText(keyboardLoginParams);
    m_keyBoardThread->SetDeviceNumber(m_deviceNo);
    m_keyBoardThread->SetGamePid(m_gamePid);

    m_keyBoardThread->start(/*controlUrl*/wsUrl ,keyboardLoginParams);
    emit this->ChangeCloudStreamerStatue("start!");
    ////start the thread protect gst-launch-1.0
    StartProtectGstLaunch();
    return 0;
}


void CloudStreamer::on_changeCloudStreamerStatue(QString statusContent){
    ui->label_16->setText(statusContent);
}

void CloudStreamer::ForeachKillExe(QString gameId){
    DealIniFile  streamConfig;
    QString executePath = QCoreApplication::applicationDirPath();
    ///路径是否合法
    if(!executePath.isEmpty()){
      {
            if(0 == streamConfig.OpenFile(executePath + "/cloudGameConfig.ini")){
                //return streamConfig.GetValue(g_iniTopic , g_gameId).toString();
                QSettings * iniParse = streamConfig.GetInitFileParse();
                if(iniParse){
                    QStringList groupList = iniParse->childGroups();
                    foreach (QString group, groupList) {
                        //streamConfig.m_iniFileParse->beginGroup(group);
                        //QStringList keyList = streamConfig.m_iniFileParse->allKeys();
                        QString game_id = streamConfig.GetValue(group, "id").toString();
                        if(0 != game_id.compare(gameId) && !game_id.isEmpty()){

                            bool IsRunning = false;
                            IsRunning = GameIsAreadlyRunning(game_id);
                            if(IsRunning){
                                KillAreadlyRunningGame(game_id);
                                LOG_INFO(QString(" ************ mjh kill the gameid =%1 ***********").arg(game_id));
                            }
                        }
                    }
                }
            }
        }
    }

}

 QString CloudStreamer::GetValueByGameID(QString gameId , QString keyName){
    if(!gameId.isEmpty() && !keyName.isEmpty()){
        QString executePath = QCoreApplication::applicationDirPath();
        ///路径是否合法
        if(!executePath.isEmpty()){
            if(!m_CloudGameConfigIniParse.get()){
                m_CloudGameConfigIniParse = std::make_shared<DealIniFile>();
                int ret =  m_CloudGameConfigIniParse->OpenFile(executePath + g_cloudStreamConfigFile);
                if(0 != ret){
                    LOG_ERROR(QString("open the file %1 is failure!").arg(g_cloudStreamConfigFile));
                    return "";
                }
            }
            QString keyValue = m_CloudGameConfigIniParse->GetValue("id" , gameId , keyName).toString();
            if(!keyValue.isEmpty()){
                return keyValue;
            }
        }
    }
    return "";
}

 bool   CloudStreamer::StartGameByGameId(QString gameId, QString startGameParams){
     if(!gameId.isEmpty()){
         QString gamePath = GetGamePathByID(gameId);
         LOG_INFO( "gamePath is parepar start!\n");
         QString params = "start;";
         std::string  strTemp = gamePath.toStdString();
         vector<string> AllStr = split(strTemp , "&&");
         if(AllStr.size() < 2){
             return false;
         }
         //params += " \"";
         params += AllStr[0].c_str();
         params += ";";
         params += AllStr[1].c_str();
         //params += "\"";
         QString appPath = QCoreApplication::applicationDirPath();
         QString str11 = appPath + "/" + g_cloudPathBat;
         LOG_INFO(QString("%1 startgame exe=%1").arg(g_cloudPathBat).arg(params));
         if(!gamePath.isEmpty()){
             bool ret = false;
             QStringList paramList= params.split(";");
             if(!startGameParams.isEmpty()){
               // ret = StartGame(str11.toLocal8Bit().data()  ,params.toLocal8Bit().data(),SW_HIDE);
                ExecuteOutterScript(str11.toLocal8Bit().data()  ,paramList);
             }else{
                 //ret = StartGame(str11.toLocal8Bit().data()  ,params.toLocal8Bit().data(),SW_HIDE);
                ExecuteOutterScript(str11.toLocal8Bit().data() , paramList);
             }
             if(ret){
                  LOG_INFO(QString("gamePath=  %1 gameId start success!\n").arg(gamePath));
                  return true;
             }else{
                  LOG_INFO(QString("gamePath= %1 gameId start failure!\n").arg(gamePath));
             }
         }else{
             LOG_INFO("gamePath is empty!\n");
         }
     }
     return false;
 }

 void CloudStreamer::ExecuteOutterScript(QString  path, const QStringList &params){
     if(!path.isEmpty()){
         QProcess  process;
         if(params.size() > 0){
            process.start(path ,params);
         }else{
            process.start(path);
         }
         process.waitForStarted(); //等待程序启动
         process.waitForFinished();//等待程序关闭
//         if(!process.waitForFinished(-1) || process.error() == FailedToStart ){
//             return;
//         }

     }
 }

 bool  CloudStreamer::GameIsAreadlyRunning(QString gameId){
     if(!gameId.isEmpty()){
        // QString gameName = GetValueByGameID(m_gameId , "gameexeName");
         QString gameName = GetValueByGameID(gameId , "gameexeName");
         if(!gameName.isEmpty()){
             // LOG_ERROR(QString("gameName:%1  gameId:%2").arg(gameName).arg(gameId));
              m_gamePid = isProcessRunning(gameName.toStdString().c_str());
              if (m_gamePid > 0) {
                LOG_ERROR(QString("mjh gameName:%1  gameId:%2  pid:%3").arg(gameName).arg(gameId).arg(m_gamePid));
              }
              return m_gamePid > 0 ? true:false;
         }
     }
     return false;
}


 bool  CloudStreamer::KillAreadlyRunningGame(QString gameId){
     if(!gameId.isEmpty()){
         //QString gameStopPath = GetValueByGameID(m_gameId , "gameExeName");//GetGameStopByID(gameId);
          QString gameStopPath = GetValueByGameID(gameId , "gameExeName");//GetGameStopByID(gameId);
         if(!gameStopPath.isEmpty()){
             KillGameByName(gameStopPath);
         }
         QString gameName = gameStopPath;//GetValueByGameID(m_gameId , "gameExeName");
         if(!gameName.isEmpty()){
             do{
                 bool result = isProcessRunning(gameName.toStdString().c_str()) > 0 ? true:false;
                 if(!result){
                     break;
                 }
                 Sleep(100);
             }while(1);
             return true;
         }
     }
     return false;
 }


QString CloudStreamer::GetGamePathByID(QString gameId){
    if(!gameId.isEmpty() ){
        QString executePath = QCoreApplication::applicationDirPath();
        ///路径是否合法
        LOG_INFO(QString("executePath:%1  gameId=%2!").arg(executePath).arg(gameId));
        if(!executePath.isEmpty()){
            if(!m_CloudGameConfigIniParse.get()){
                m_CloudGameConfigIniParse = std::make_shared<DealIniFile>();
                int ret =  m_CloudGameConfigIniParse->OpenFile(executePath + g_cloudStreamConfigFile);
                if(0 != ret){
                   LOG_ERROR(QString("the file is open %1 failure!").arg(executePath + g_cloudStreamConfigFile));
                   return "";
                }
            }
            QString path = m_CloudGameConfigIniParse->GetValue("id" , gameId , "path").toString();
            LOG_INFO(QString("gamePath :%1  gameId=%2!").arg(path).arg(gameId));
            if(!path.isEmpty()){
                return  path;
            }
        }
    }
    return "";
}


QString CloudStreamer::GetGameStopByID(QString gameId){
    if(!gameId.isEmpty() ){
        QString executePath = QCoreApplication::applicationDirPath();
        ///路径是否合法
        if(!executePath.isEmpty()){
            if(!m_CloudGameConfigIniParse.get()){
                m_CloudGameConfigIniParse = std::make_shared<DealIniFile>();
                int  ret = m_CloudGameConfigIniParse->OpenFile(executePath + g_cloudStreamConfigFile);
                if(0 != ret){
                   LOG_ERROR(QString("open %1 is failure!").arg(executePath + g_cloudStreamConfigFile));
                   return "";
                }
            }
            // QString path = streamConfig.GetValue("id" , gameId , "stopScript").toString();
            QString path = m_CloudGameConfigIniParse->GetValue("id" , gameId , "gameExeName").toString();
            if(!path.isEmpty()){
                return  path;
            }
        }
    }
    return "";
}

void CloudStreamer::StopGameCallback(QString data){
    LOG_INFO("enter StopGameCallback  !");
    //std::unique_lock<std::mutex> lock(m_gameMutex);
    LastGaspGoalPushStreamer();
    QString gameName = GetValueByGameID(m_gameId , "gameExeName");
    RecordGameInfo *recordInfos1 = RecordGameInfo::GetInstance();
    recordInfos1->RecordInfo(m_gameId , 0);
    QString  testStr = WSServiceTransferSignStringEx(m_deviceNo , m_sessionId , m_gameId ,gameName);
    StopProtectGstLaunch();
    emit m_cloudGameServiceIterator->Send(testStr);
    //emit ui->pushButton_3->clicked();
    //////////close push Streamer and  keyboard affair
//    QString roomName = m_pushStreamerParams.m_roomId;
//    QString pushServer = ui->lineEdit->text();
//    std::string roomNameStr = roomName.toStdString();
//    std::string pushServerStr = pushServer.toStdString();
    if(UI_MODE::DEFAULT_MODE == m_mode || UI_MODE::ONLY_PUSH_STREAMER == m_mode){
        LOG_INFO(QString("StopGameCallback roomId = %1   serverUrl= %2 !").arg(m_pushStreamerParams.m_roomId.c_str()).arg(m_pushStreamerParams.m_serverUrl.c_str()));
        closepush(const_cast<char*>(m_pushStreamerParams.m_roomId.c_str()),const_cast<char*> (m_pushStreamerParams.m_serverUrl.c_str()));
    }
    ///////
    if(UI_MODE::DEFAULT_MODE == m_mode || UI_MODE::ONLY_KEY_BOARD == m_mode){
        if(m_keyBoardThread.get()){
            m_keyBoardThread->stop();
            disconnect(m_keyBoardThread.get() , SIGNAL(RecordSignal(QString , QString)) , this , SLOT(RecordSignalCallBack(QString , QString )));
            m_keyBoardThread.reset();
        }
    }
    ////////
    if(!data.isEmpty()){
        Json::Reader reader;
        Json::Value root;
        root["init"] = 123;
        if (reader.parse(data.toStdString(), root)){
           if(root.isMember("isForce")){
                int isForce = root["isForce"].asInt();
                if(1 == isForce){//强制关闭
                    QString gamePath = GetValueByGameID(m_gameId , "gameExeName");//GetGameStopByID(m_gameId);
                    if(!gamePath.isEmpty()){
                        KillGameByName(gamePath);
                        ActiveReportGameStatus();
                        LOG_INFO( "game stop success!\n");
                    }
                }
           }
           QString gameId = root["gameId"].asCString();
           QString roomId = root["roomId"].asCString();
           QString serverUrl = root["serverUrl"].asCString();
           QString playerUrl = root["playerUrl"].asCString();
          // QString peerId = root["peerId"].asCString();
          // QString videoPort = root["videoPort"].asCString();
           QString controlUrl = root["controlUrl"].asCString();
           QString port = root["port"].asCString();
           //QString keyboardPort = root["keyboardPort"].asCString();
           ///////////////https://124.71.159.87:4443/
           int ret =  -1;
           if(serverUrl.isEmpty()|| controlUrl.isEmpty() || port.isEmpty()){
               LOG_INFO("StopGameCallback  param format failure!");
               return ;
           }
           ///
           if(gameId.isEmpty()){
                LOG_INFO("gameId or  startGameParams is empty!\n");
           }
           QString gamePath = GetValueByGameID(m_gameId , "gameExeName");//GetGameStopByID(gameId);
           if(!gamePath.isEmpty()){
               KillGameByName(gamePath);
               ActiveReportGameStatus();
               LOG_INFO("game stop success!\n");
           }
        }
    }
}

void CloudStreamer::DownloadCallback(QString downloadUrl , QString savePath){
    //QMessageBox::information(NULL , "information!" , "DownloadCallback");
}

void CloudStreamer::PushStreamCallback(QString streamParams){
    //QMessageBox::information(NULL , "information!" , "PushStreamCallback");
}

void CloudStreamer::CloseStreamCallback(QString streamParams){
    //QMessageBox::information(NULL , "information!" , "CloseStreamCallback");
}

int generateRandomNumber(){
        int tag[]={0,0,0,0,0,0,0,0,0,0};
        int four=0;
        int temp=10;

        while(four<1000){
            //设置随机数种子,否则每次得到的随机数都相同使得"不随机"
            qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
            temp=qrand()%10;//随机获取0~9的数字
            if(tag[temp]==0){
                four+=temp;
                four*=10;
                tag[temp]=1;
            }
        }
        return four;
}

QString  WSServiceTransferSignString(QString &deviceNo){
    ///
    Json::Value root;
    Json::Value data;
    ////////
    int ret = -1;
    QString noncestrQt = QString("%1").arg(generateRandomNumber());
    root["noncestr"] = noncestrQt.toLocal8Bit().data();

    QDateTime dateTime = QDateTime::currentDateTime();
    QString timestamp = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
    //int ms = dateTime.time().msec();
    qint64 epochTime = dateTime.toMSecsSinceEpoch();
    QString epochTimeStr = QString("%1").arg(epochTime);
    root["timestamp"] = epochTimeStr.toLocal8Bit().data();
    QString signStr = epochTimeStr  + g_signKey +  noncestrQt;
    QString signStrMd5 = QCryptographicHash::hash(signStr.toLatin1(),QCryptographicHash::Md5).toHex();
    root["sign"] =signStrMd5.toLocal8Bit().data();
    root["type"] = "SignIn";
    root["sessionId"] = "";
    ////////
    QString deviceNoStr = "";//noncestrQt + epochTimeStr;
    FILE * file = NULL;
    file = fopen("c:\\testDeviceNo.txt" , "r");
    if(file){
        //求得文件的大小
        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        rewind(file);
        //申请一块能装下整个文件的空间
        char* ar = (char*)malloc(sizeof(char)*size);
        //读文件
        fread(ar,1,size,file);//每次读一个，共读size次
        ar[size] = '\0';
        deviceNoStr = ar;
        //free(ar);
        fclose(file);
    }
    deviceNo = deviceNoStr;
    data["deviceNo"] = deviceNoStr.toLocal8Bit().data();
    root["data"] = data;
    ////////
    Json::FastWriter styled_write;
    std::string rootJsonStr = styled_write.write(root);
    return QString("%1").arg(rootJsonStr.c_str());
}




QString  WSServiceTransferSignStringEx(QString deviceNo, QString sessionId , QString gameId , QString gamePath){
    ///
    Json::Value root;
    Json::Value data;
    ////////
    int ret = -1;
    QTime time;
    time= QTime::currentTime();
    qsrand(time.msec()+time.second()*1000);
    QString noncestrQt = QString("%1").arg(qrand());
    root["noncestr"] = noncestrQt.toLocal8Bit().data();

    QDateTime dateTime = QDateTime::currentDateTime();
    QString timestamp = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
    //int ms = dateTime.time().msec();
    qint64 epochTime = dateTime.toMSecsSinceEpoch();
    QString epochTimeStr = QString("%1").arg(epochTime);
    root["timestamp"] = epochTimeStr.toLocal8Bit().data();
    QString signStr = epochTimeStr  + g_signKey +  noncestrQt;
    QString signStrMd5 = QCryptographicHash::hash(signStr.toLatin1(),QCryptographicHash::Md5).toHex();
    root["sign"] =signStrMd5.toLocal8Bit().data();
    root["type"] = "GameReportDeviceState";
    root["sessionId"] = sessionId.toStdString().c_str();
    data["deviceNo"] = deviceNo.toLocal8Bit().data();
    data["gameId"] = gameId.toStdString().c_str();
    int gameIsRunning = 0 ;
    ///////////////
    RecordGameInfo *recordInfos1 = RecordGameInfo::GetInstance();
    gameIsRunning = recordInfos1->GetGameStatus();
    data["status"] = gameIsRunning ? 1 : 0;
    data["pushVersion"] = "1.0.1.18";
    //////////////////
    root["data"] = data;
    ////////
    Json::FastWriter styled_write;
    std::string rootJsonStr = styled_write.write(root);
    return QString("%1").arg(rootJsonStr.c_str());
}




void CloudStreamer::SignInWsService(){
    if(m_cloudGameServiceIterator.get()){
        QString  signInStr =  WSServiceTransferSignString(m_deviceNo);
        LOG_INFO(QString("SignInWsService success %1").arg(signInStr));


        emit m_cloudGameServiceIterator->Send(signInStr);
        //emit m_cloudGameServiceIterator->Send(testStr);
    }
}


void CloudStreamer::ActiveReportGameStatus(){
    QString gameName = GetValueByGameID(m_gameId , "gameExeName");
    QString  testStr = WSServiceTransferSignStringEx(m_deviceNo , m_sessionId , m_gameId ,gameName);
    emit m_cloudGameServiceIterator->Send(testStr);
    LOG_INFO( QString("send GameReportDeviceState status: %1").arg(testStr));
}

void CloudStreamer::SignInCloudGameCallback(QString paramData){
    if(paramData.isEmpty()){
        return;
    }
    Json::Reader reader;
    Json::Value root;
    root["init"]=123;
    if (reader.parse(paramData.toStdString(), root)){
        ////////////
        if(root.isMember("sessionId")){
            if(root["sessionId"].isString()){
                    m_sessionId = root["sessionId"].asCString();
                    ///////signIn succes then recovery the game status
                    RecoveryGame();
            }
            //QString data = "{\"clientType\":\"2\",\"type\":\"SignIn\"}";
//            if(m_cloudGameServiceIterator.get()){
//                emit m_cloudGameServiceIterator->Send(QString("%1").arg(rootJsonStr.c_str()));
//            }
            LOG_INFO( QString("SignInCloudGameCallback success %1!").arg(paramData));
        }
    }else{
        LOG_INFO("SignInCloudGameCallback parse json  failure !");
    }

    //StartGameByGameId("gmly2" , "");
    //////
}


QString CloudStreamer::ModifiyStartParams(QString framerate, QString bitrate,QString deadline){
    QString result = "";
    if(!framerate.isEmpty() && !bitrate.isEmpty() && !deadline.isEmpty()){
        RecordGameInfo *recordInfos = RecordGameInfo::GetInstance();
        QString rootJsonStr = recordInfos->GetGameInfo();
        if(!rootJsonStr.isEmpty()){
            Json::Reader reader;
            Json::Value root;
            root["init"] = 123;
            if (reader.parse(rootJsonStr.toStdString(), root)){
                root["framerate"] = framerate.toLocal8Bit().data();
                root["bitrate"] = bitrate.toLocal8Bit().data();
                root["deadline"] = deadline.toLocal8Bit().data();
                Json::FastWriter styled_write;
                result = styled_write.write(root).c_str();
                recordInfos->RecordInfo(result);
            }
        }
    }
    return result;
}

void CloudStreamer::ChangeResolutionCallback(QString paramData){
    if(paramData.isEmpty()){
        LOG_ERROR("ChangeResolutionCallback paramData is Empty!");
        return;
    }
    Json::Reader reader;
    Json::Value root;
    QString framerate = "";
    QString bitrate="";
    QString deadline="";
    root["init"] = 123;
    if (reader.parse(paramData.toStdString(), root)){
        ////////////
        if(root.isMember("framerate")){
            if(root["framerate"].isString()){
                framerate = root["framerate"].asCString();
            }
        }

        if(root.isMember("bitrate")){
            if(root["bitrate"].isString()){
                bitrate = root["bitrate"].asCString();
            }
        }

        if(root.isMember("deadline")){
            if(root["deadline"].isString()){
                deadline = root["deadline"].asCString();
            }
        }

    }else{
        LOG_ERROR("ChangeResolutionCallback parse json  failure !");
        return;
    }
    QString newStartParams ="";
    newStartParams = ModifiyStartParams(framerate , bitrate , deadline);
    if(!newStartParams.isEmpty()){
        RecordGameInfo *recordInfos = RecordGameInfo::GetInstance();
        int gameStatus = recordInfos->GetGameStatus();
        //QString rootJsonStr = recordInfos->GetGameInfo();
        if(gameStatus){
            ///////game is not close normally , then recovery the normal show
            ///if the game's exe is running then show the gameWindow top modal
            ///if the game's exe is doesn't running , then restart the game's exe
            LOG_INFO("start ChangeResolutionCallback !");
            StopWorkThread(m_startGameThread);
            m_startGameThread = std::make_shared<std::thread>(&CloudStreamer::StartGameCallback, this , newStartParams , ONLY_PUSHSTREAMR);
            if(m_startGameThread.get()){
                m_startGameThread->detach();
            }
        }
    }
}


void CloudStreamer::MessageFeedBack(QString msgType ,  QString resultCode , QString resultMsg){
    if(!msgType.isEmpty()){
        ///
        Json::Value root;
        Json::Value data;
        ////////
        int ret = -1;
        QTime time;
        time= QTime::currentTime();
        qsrand(time.msec()+time.second()*1000);
        QString noncestrQt = QString("%1").arg(qrand());
        root["noncestr"] = noncestrQt.toLocal8Bit().data();

        QDateTime dateTime = QDateTime::currentDateTime();
        QString timestamp = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
        //int ms = dateTime.time().msec();
        qint64 epochTime = dateTime.toMSecsSinceEpoch();
        QString epochTimeStr = QString("%1").arg(epochTime);
        root["timestamp"] = epochTimeStr.toLocal8Bit().data();
        QString signStr = epochTimeStr  + g_signKey +  noncestrQt;
        QString signStrMd5 = QCryptographicHash::hash(signStr.toLatin1(),QCryptographicHash::Md5).toHex();
        root["sign"] =signStrMd5.toLocal8Bit().data();
        root["type"] = "ReceiveCallback";
        root["sessionId"] = m_sessionId.toStdString().c_str();
        //////////////data area
        data["receiveType"] = msgType.toStdString().data();
        data["resultCode"] = resultCode.toStdString().c_str();
        data["resultMessage"] = resultMsg.toStdString().c_str();
        //////////////////
        root["data"] = data;
        ////////
        Json::FastWriter styled_write;
        std::string rootJsonStr = styled_write.write(root);
        QString result = rootJsonStr.c_str();
        LOG_INFO(QString("MessageFeedBack: %1 ").arg(result));
        emit m_cloudGameServiceIterator->Send(result);
    }
}


void    CloudStreamer::StartReportStatusTimer(){
    LOG_INFO(QString("g_reportGameStatusInteral =%1").arg(g_reportGameStatusInteral));
    m_gameStatusTimer = new QTimer(0);
    m_gameStatusTimer->setInterval(g_reportGameStatusInteral);

    m_gameStatusThread = new QThread();
    m_gameStatusTimer->moveToThread(m_gameStatusThread);

    connect(m_gameStatusThread , SIGNAL(finished()) , m_gameStatusTimer, SLOT(deleteLater()));
    connect(m_gameStatusThread  , SIGNAL(started()) , m_gameStatusTimer, SLOT(start()));
    connect(m_gameStatusTimer , SIGNAL(timeout()) , this , SLOT(GameStatusCallback()));
    m_gameStatusThread->start();
    ///////////
    ActiveReportGameStatus();
}

void    CloudStreamer::StopReportStatusTimer(){
    if(m_gameStatusTimer){
        disconnect(m_gameStatusTimer , SIGNAL(timeout()) , this , SLOT(GameStatusCallback()));
    }
    if(m_gameStatusThread){
        if(!m_gameStatusThread->isFinished()){
            if(m_gameStatusThread->isRunning()){
                m_gameStatusThread->quit();
                m_gameStatusThread->wait();
            }
        }
        delete m_gameStatusThread;
        m_gameStatusThread= NULL;
    }

//    if(m_gameStatusTimer && m_gameStatusTimer->isActive()){
//        m_gameStatusTimer->stop();
//        m_gameStatusTimer->deleteLater();
//    }
}

void CloudStreamer::StartAutoUpdate(){
    return;
    QString updateScript = "";
    QString executePath = QCoreApplication::applicationDirPath();
    if(executePath.isEmpty()){
        LOG_ERROR("executePath should be empty!");
        return;
    }
    updateScript = executePath + g_autoUpdateScript;
    QString tempStr = "\"start\"";
    StartGame(updateScript.toLocal8Bit().data() , tempStr.toLocal8Bit().data());
}

void CloudStreamer::CloseAutoUpdate(){
    return;
    QString updateScript = "";
    QString executePath = QCoreApplication::applicationDirPath();
    if(executePath.isEmpty()){
        LOG_ERROR("executePath should be empty!");
        return;
    }
    updateScript = executePath + g_autoUpdateScript;
    QString tempStr = "\"stop\"";
    StartGame(updateScript.toLocal8Bit().data() , tempStr.toLocal8Bit().data());
}

void CloudStreamer::RecordSignalCallBack(QString flagStr , QString logStr){
    LOG_INFO(QString("%1  %2").arg(flagStr).arg(logStr));
}

void CloudStreamer::ParseMessageCallback(QString data){
    Json::Reader reader;
    Json::Value root;
    root["haitianyise"]="just for test";
    LOG_INFO(QString("Parsemsg:%1" ).arg(data));
    if (reader.parse(data.toStdString(), root)){
        bool exsit = root.isMember("type");
        if(exsit){
            std::string msgType = root["type"].asCString();
            std::string returnCodeStr = root["code"].asCString();
            int returnCode = atoi(returnCodeStr.c_str());
            if( 0  != returnCode){
                LOG_ERROR("json format failure  code is not equal 0!");
                return ;
            }
            if(!msgType.empty()){
                ////////parse message  according msgType
                bool existData = root.isMember("data");
                if(existData){
                    Json::FastWriter styled_write;
                    std::string rootJsonStr = styled_write.write(root["data"]);
                   if(0 == msgType.compare("StartGame")){//start game
                        StopWorkThread(m_startGameThread);
                        m_startGameThread = std::make_shared<std::thread>(&CloudStreamer::StartGameCallback, this , rootJsonStr.c_str(),NORMAL_MODEL);
                        if(m_startGameThread.get()){
                            m_startGameThread->detach();
                        }
                    }else if(0 ==  msgType.compare("StopGame")){//stop game
                       StopWorkThread(m_stopGameThread);
                       m_stopGameThread = std::make_shared<std::thread>(&CloudStreamer::StopGameCallback, this , rootJsonStr.c_str());
                       if(m_stopGameThread.get()){
                           m_stopGameThread->detach();
                       }
                   }else if(0 == msgType.compare("SignIn") ){//signIn
                       StopWorkThread(m_signInThread);
                       m_signInThread = std::make_shared<std::thread>(&CloudStreamer::SignInCloudGameCallback , this , rootJsonStr.c_str());
                       if(m_signInThread.get()){
                           m_signInThread->detach();
                       }
                       //addLogToEdit(UI_INFO , "maybe should go!");
                       StopReportStatusTimer();
                       StartReportStatusTimer();
                   }else if(0 == msgType.compare("ChangeResolution")){
                       StopWorkThread(m_changeResolution);
                       m_changeResolution = std::make_shared<std::thread>(&CloudStreamer::ChangeResolutionCallback , this , rootJsonStr.c_str());
                       if(m_changeResolution.get()){
                           m_changeResolution->detach();
                       }
                       LOG_INFO("ChangeResolution message!");
                   }
                }
            }
        }else{
            bool exsit = root.isMember("code");
            if(exsit){
                std::string codeStr = root["code"].asString();
                if(codeStr.compare("0") == 0){
                    SignInWsService();
                }else{

                }
            }
        }
    }else{
         LOG_ERROR("ParseMessageCallback parse json  failure !");
    }
}

void CloudStreamer::on_comboBox_4_currentIndexChanged(int index)
{
    SetUIModel((UI_MODE)(index));
}
