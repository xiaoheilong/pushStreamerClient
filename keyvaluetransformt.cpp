#include "keyvaluetransformt.h"

#include <WinSock2.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <memory>
#include <iostream>
#include "CloudGame.h"
#include <windows.h>
namespace KeyValueTransformtNamespace{
////range 为后面映射后的数据的范围
float GetPercentInRange(float value , float  left , float right,  float range){
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
    return difference * range;
}

int NSSleep(int intel)  // ms
{
    HANDLE hTimer = NULL;
    LARGE_INTEGER liDueTime;

    liDueTime.QuadPart = -1 * intel * 10000;

    // Create a waitable timer.
    hTimer = CreateWaitableTimer(NULL, TRUE, L"WaitableTimer");
    if (!hTimer)
    {
        printf("CreateWaitableTimer failed (%d)\n", GetLastError());
        return 1;
    }

    // Set a timer to wait for 10 seconds.
    if (!SetWaitableTimer(
                hTimer, &liDueTime, 0, NULL, NULL, 0))
    {
        printf("SetWaitableTimer failed (%d)\n", GetLastError());
        return 2;
    }

    // Wait for the timer.
    if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
        printf("WaitForSingleObject failed (%d)\n", GetLastError());

    return 0;
}

////////////
KeyValueTransformt::KeyValueTransformt(QString nameKeyTable , QString defaultKeyBoardPath , QString gameKeyBoardPath): m_lastDirect(DIRECT_ORIGIN)
{
    m_nameKeyTable = std::make_shared<DealIniFile>();
    m_nameKeyTable->OpenFile(nameKeyTable);

    m_defaultKeyBoardPath = std::make_shared<DealIniFile>();
    m_defaultKeyBoardPath->OpenFile(defaultKeyBoardPath);

    m_gameKeyBoardPath = std::make_shared<DealIniFile>();
    m_gameKeyBoardPath->OpenFile(gameKeyBoardPath);
    m_consumerKeyboard = std::make_shared<ConsumerKeyBoardThread>();
    m_consumerKeyboard->start();

    m_lastDirectPoint.x = 0;
    m_lastDirectPoint.y = 0 ;
}


KeyValueTransformt::~KeyValueTransformt(){
    if(m_nameKeyTable.get()){
        m_nameKeyTable.reset();
    }
    if(m_defaultKeyBoardPath.get()){
        m_defaultKeyBoardPath.reset();
    }

    if(m_gameKeyBoardPath.get()){
        m_gameKeyBoardPath.reset();
    }

    m_keyValueMap.clear();

    if(m_consumerKeyboard.get()){
        m_consumerKeyboard->Stop();
        m_consumerKeyboard.reset();
    }
    m_lastDirectPoint.x = 0;
    m_lastDirectPoint.y = 0 ;
    //MessageBoxA(NULL, "quit", "keyValueTransformat!" , MB_OK);
}


void KeyValueTransformt::ReverseBounce(float x , float y){
    if(0 == m_lastDirectPoint.x && 0 == m_lastDirectPoint.y){
        return;
    }
    bool xDirectSame = false;
    bool yDirectSame = false;
    if( (0 < x && 0 < m_lastDirectPoint.x) || (0 > x && 0 > m_lastDirectPoint.x )){
           xDirectSame = true;
    }
    if((0 < y && 0 < m_lastDirectPoint.y) || (0 > y && 0 > m_lastDirectPoint.y )){
           yDirectSame = true;
    }
    if(xDirectSame && yDirectSame){//相同方向不做处理
        return;
    }
    std::vector<int> vector;
    int ret = -1;
    DirectionKey direction ;
    if(!xDirectSame){
        direction  =m_lastDirectPoint.x > 0 ? DIRECT_RIGHT:DIRECT_LEFT;
        ret = GetMapKeyValueEx(direction , vector);
    }

    if(!yDirectSame){
        direction  =m_lastDirectPoint.y > 0 ? DIRECT_DOWN:DIRECT_TOP;
        ret = GetMapKeyValueEx(direction , vector);
    }
    for(int i =0; i < vector.size(); ++i){
            int value = vector[i];
            Products  callback;
            callback.m_type = ConsumerKeyboardValueSpace::KeyBoardType::HANDLE_SHAKE_DIRECT_KEY;
            callback.m_pointer =nullptr;
            callback.m_callback = std::bind([value](){
                if(!KeyUp(value)){
                    NSSleep(50);
                    KeyUp(value);
                }else{
                    NSSleep(1);
                }
            });
            AddKeyValueToThread(callback);
    }

}

 int KeyValueTransformt::ConvertXYDirection(float x , float y , int ud ){
    std::vector<int> vector;
    int ret = -1;
    DirectionKey direction ;
    if(!m_nameKeyTable.get() &&!m_defaultKeyBoardPath.get() ){
        m_lastDirectPoint.x = 0;
        m_lastDirectPoint.y = 0;
        return ret;
    }
    if(0 == x && 0 == y ){
        ret = -2;
        m_lastDirectPoint.x = 0;
        m_lastDirectPoint.y = 0;
        return ret;
    }else{
        if( 0  <  x &&  0 == y ){
            direction = DIRECT_RIGHT;
        }else  if ( 0 < x  && 0 <  y ){
            direction =DIRECT_RIGHT_DOWN;
        }else if( 0 == x &&  0 < y){
            direction  = DIRECT_DOWN;
        }else if( 0 > x && 0 > y ){
            direction = DIRECT_LEFT_TOP;
        }else if(0 > x  && 0 == y){
            direction = DIRECT_LEFT;
        }else if(0 > x  && 0 < y){
            direction = DIRECT_LEFT_DOWN;
        }else if(0  == x && 0 > y){
            direction = DIRECT_TOP;
        }else if(0 < x && 0 > y){
            direction = DIRECT_TOP_RIGHT;
        }
        if(DirectionKey::DIRECT_ORIGIN != direction){
            ret = GetMapKeyValueEx(direction , vector);
        }
    }
    ReverseBounce(x , y);
    m_lastDirect = direction;
    m_lastDirectPoint.x = x;
    m_lastDirectPoint.y = y;
    if(0 == ret){
        for(int i =0; i < vector.size(); ++i){
                int value = vector[i];
                Products  callback;
                callback.m_type = ConsumerKeyboardValueSpace::KeyBoardType::HANDLE_SHAKE_DIRECT_KEY;
                callback.m_pointer = nullptr;
                callback.m_callback =  std::bind([value](){
                    if(!KeyDown(value)){
                        KeyUp(value);
                        KeyDown(value);
                    }else{
                        NSSleep(1);
                    }
                });
                AddKeyValueToThread(callback);
        }
    }
    return ret;
}

void KeyValueTransformt::ConvertMouse(float &z , float &rz){
    z = GetPercentInRange(z , -1 , 1 , 32737);
    rz = GetPercentInRange(rz , -1 , 1 , 32737);
}


void KeyValueTransformt::DirectionAllUp(){
    if(m_consumerKeyboard.get()){
        return;
    }
    std::vector<int> vector;
    GetMapKeyValueEx(m_lastDirect , vector);
    for(int i =0; i < vector.size(); ++i){
          int value = vector[i];
          Products  callback;
          callback.m_type = ConsumerKeyboardValueSpace::KeyBoardType::HANDLE_SHAKE_DIRECT_KEY;
          callback.m_pointer = nullptr;
          callback.m_callback =  std::bind([value](){
              if(!KeyUp(value)){
                 NSSleep(50);
                 KeyUp(value);
              }else{
                 NSSleep(1);
              }
          });
          AddKeyValueToThread(callback);
    }
}

int KeyValueTransformt::KeyDown_C(int keyValue){
    if(!m_nameKeyTable.get() &&!m_defaultKeyBoardPath.get() ){
        return -1;
    }
    int ret = 0;
    QString result = GetMapKeyValue(QString("%1").arg(keyValue));
    QStringList resultArray = result.split(":");//
    int size = resultArray.size();
    for(int index = 0; index < size;++index ){
        int key = resultArray[index].toInt();
        if(key){
            Products  callback;
            callback.m_type = ConsumerKeyboardValueSpace::KeyBoardType::NORMAL_KEY_ACTION;
            callback.m_pointer = nullptr;
            callback.m_callback = std::bind([key](){
                if(1 !=key && 2 != key && 3 != key){
                    if(!KeyDown(key)){
                       KeyUp(key);
                       KeyDown(key);
                    }else{
                        NSSleep(1);
                    }
                }else{
//                    QString logStr = QString("MouseDown key=%1").arg(key);
//                    MessageBoxA(NULL , logStr.toStdString().data() , "just for fun" , MB_OK);
                    if(!MouseDown(16383, 16383 , key)){
                        NSSleep(50);
                        MouseDown(16383, 16383 , key);
                    }else{
                        NSSleep(1);
                    }
                }
            });
            AddKeyValueToThread(callback);
        }
    }
    return ret;
}

int KeyValueTransformt::KeyUP_C(int keyValue){
    if(!m_nameKeyTable.get() &&!m_defaultKeyBoardPath.get() ){
        return -1;
    };
    int ret = 0;
    QString result = GetMapKeyValue(QString("%1").arg(keyValue));
    QStringList resultArray = result.split(":");//
    int size = resultArray.size();
    for(int index = 0; index < size;++index ){
        int key = resultArray[index].toInt();
        if(key){
//            if(!KeyUp(key)){
//                ret = -1;
//            }
            Products  callback;
            callback.m_type = ConsumerKeyboardValueSpace::KeyBoardType::NORMAL_KEY_ACTION;
            callback.m_pointer = nullptr;
            callback.m_callback = std::bind([key](){
                if(1 !=key && 2 != key && 3 != key){
                    if(!KeyUp(key)){
                        NSSleep(50);
                        KeyUp(key);
                    }else{
                        NSSleep(1);
                    }
                }else{
//                    QString logStr = QString("MouseUp key=%1").arg(key);
//                    MessageBoxA(NULL , logStr.toStdString().data() , "just for fun" , MB_OK);
                    if(!MouseUp(16383, 16383 , 0)){
                        NSSleep(50);
                        MouseUp(16383, 16383 , 0);
                    }else{
                        NSSleep(1);
                    }
                }
            });
            AddKeyValueToThread(callback);
        }
    }
    return ret;
}
//鼠标移动
int KeyValueTransformt::MouseMove_C(int x, int y, int code){
    if(!m_nameKeyTable.get() &&!m_defaultKeyBoardPath.get()){
        return -1;
    }
//    if(!MouseMove(x, y ,code)){
//        if(!MouseMove(x, y ,code)){
//            return -1;
//        }
//    }

    Products  callback;
    callback.m_type = ConsumerKeyboardValueSpace::KeyBoardType::MOUSE_ACTION;
    callback.m_pointer = nullptr;
    callback.m_callback = std::bind([x, y, code](){
        if(!MouseMove(x, y ,code)){
            NSSleep(50);
            MouseMove(x, y ,code);
        }else{
            NSSleep(1);
        }
    });
    AddKeyValueToThread(callback);
    return 0;
}
//鼠标抬起
int KeyValueTransformt::MouseUp_C(int x, int y, int code){
    if(!m_nameKeyTable.get() &&!m_defaultKeyBoardPath.get() ){
        return -1;
    }
//   if( !MouseUp(x, y , code)){
//        return -1;
//   }
    Products  callback;
    callback.m_type = ConsumerKeyboardValueSpace::KeyBoardType::MOUSE_ACTION;
    callback.m_pointer =nullptr;
    callback.m_callback = std::bind([x, y, code](){
        if(!MouseUp(x, y , code)){
            NSSleep(50);
            MouseUp(x, y , code);
        }else{
            NSSleep(1);
        }
    });
    return 0;
}
//鼠标按下
int KeyValueTransformt::MouseDown_C(int x, int y, int code){
    if(!m_nameKeyTable.get() &&!m_defaultKeyBoardPath.get() ){
        return -1;
    }
//    if(!MouseDown(x, y , code)){
//        return -1;
//    }

    Products  callback;
    callback.m_type = ConsumerKeyboardValueSpace::KeyBoardType::MOUSE_ACTION;
    callback.m_pointer = nullptr;
    callback.m_callback = std::bind([x , y , code](){
        if(!MouseDown(x, y , code)){
            NSSleep(50);
            MouseDown(x, y , code);
        }else{
            NSSleep(1);
        }
    });
    AddKeyValueToThread(callback);
   return 0;
}

QString KeyValueTransformt::GetKeyValueInMap(QString orignalKey){
    QString ret = "";
    if(!orignalKey.isEmpty()){
        std::string keyStr = orignalKey.toLocal8Bit().data();
        if(m_keyValueMap.count(keyStr)){
            ret = m_keyValueMap[keyStr].c_str();
        }
    }
    return ret ;
}

int KeyValueTransformt::SetKeyValueInMap(QString key , QString value){
    int ret = -1;
    if(!key.isEmpty()){
        std::string keyStr = key.toLocal8Bit().data();
        m_keyValueMap[keyStr] = value.toLocal8Bit().data();
    }else{
        m_keyValueMap.insert(std::make_pair<std::string,std::string >(key.toLocal8Bit().data(), value.toLocal8Bit().data()));
    }
    return ret ;
}

void KeyValueTransformt::AddKeyValueToThread(Products callback){
    if(m_consumerKeyboard){
        m_consumerKeyboard->Produce(callback);
    }

}


int KeyValueTransformt::GetMapKeyValueEx(DirectionKey direction , std::vector<int> &vector){

    QString  orignalKey;
    switch(direction){
        case DIRECT_RIGHT:{
            orignalKey = "RIGHT";
        }
            break;
        case DIRECT_RIGHT_DOWN:{
            orignalKey = "DOWN_RIGHT";
        }
            break;
        case DIRECT_DOWN:{
            orignalKey = "DOWN";
        }
            break;
        case DIRECT_LEFT_DOWN:{
            orignalKey = "DOWN_LEFT";
        }
            break;
        case DIRECT_LEFT:{
            orignalKey = "LEFT";
        }
            break;
        case DIRECT_LEFT_TOP:{
            orignalKey = "UP_LEFT";
        }
            break;
        case DIRECT_TOP:{
            orignalKey = "UP";
        }
            break;
        case DIRECT_TOP_RIGHT:{
            orignalKey = "UP_RIGHT";
        }
        break;
        default:
        {

        }
        break;
    }

    QString resultValue  = GetKeyValueInMap(orignalKey);
    if(!resultValue.isEmpty()){
        QStringList resultList = resultValue.split(":");
        for(int index =0 ; index < resultList.size();++index){
            vector.push_back(resultList[index].toInt());
        }
        return 0;
    }

    if(!m_nameKeyTable.get() &&!m_defaultKeyBoardPath.get() ){
        return -1;
    }
    int ret = -1;
    if(!orignalKey.isEmpty() && m_nameKeyTable.get() && m_defaultKeyBoardPath.get() ){
            QString keyName = "";
            QString defaultKeyValue ="";
            if(m_gameKeyBoardPath.get()){
                defaultKeyValue = m_gameKeyBoardPath->GetValue("KeyboardMapValue" , orignalKey).toString();
            }

            if(defaultKeyValue.isEmpty()){
                defaultKeyValue =  m_defaultKeyBoardPath->GetValue("KeyboardMapValue" , orignalKey).toString();
            }
            if(!defaultKeyValue.isEmpty()){//默认按键配置文件中有定义目标name的key value
                ret = 0;
                SetKeyValueInMap(orignalKey, defaultKeyValue);
                QStringList resultList = defaultKeyValue.split(":");
                for(int index =0 ; index < resultList.size();++index){
                    vector.push_back(resultList[index].toInt());
                }
            }
    }

    return ret;
}


QString  KeyValueTransformt::GetMapKeyValue(QString keyName){
    if(!m_nameKeyTable.get() &&!m_defaultKeyBoardPath.get() ){
        return "";
    }
    if(keyName.isEmpty()){
        return "";
    }
    QString ret = "";

    ret = GetKeyValueInMap(keyName);
    if(!ret.isEmpty()){
        return ret;
    }

    QString keyName1 = "";
    keyName1 = m_nameKeyTable->GetValue("KeyboardMapValue" , keyName).toString();
    if(!keyName1.isEmpty()){
        QString defaultKeyValue ="";
        if(m_gameKeyBoardPath.get()){
            defaultKeyValue = m_gameKeyBoardPath->GetValue("KeyboardMapValue" , keyName1).toString();
        }
        if(defaultKeyValue.isEmpty()){//游戏按键配置文件中没有定义目标name的key value
            defaultKeyValue =  m_defaultKeyBoardPath->GetValue("KeyboardMapValue" , keyName1).toString();
            if(!defaultKeyValue.isEmpty()){//默认按键配置文件中有定义目标name的key value
                ret = defaultKeyValue;
            }
        }else{////游戏按键配置文件中有定义目标name的key value
            ret =defaultKeyValue;//.toInt();
        }
    }

    if(!ret.isEmpty()){
        SetKeyValueInMap(keyName , ret);
    }

    return ret;


}

}
