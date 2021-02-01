#ifndef KEYVALUETRANSFORMT_H
#define KEYVALUETRANSFORMT_H
#include "dealinifile.h"
#include <memory>
#include <QString>
#include <vector>
#include <map>

#include "consumerkeyboardvalue.h"
namespace KeyValueTransformtNamespace{
using namespace DealIniFileSpace;
//typedef void(*keyCallback1)();
typedef ConsumerKeyboardValueSpace::Products Products;
typedef ConsumerKeyboardValueSpace::ConsumerKeyBoardValue<Products>  ConsumerKeyBoardThread;
//typedef DealIniFileSpace::DealIniFile DealIniFile;
enum  DirectionKey{
    DIRECT_ORIGIN = 0,
    DIRECT_RIGHT ,
    DIRECT_RIGHT_DOWN,
    DIRECT_DOWN,
    DIRECT_LEFT_DOWN,
    DIRECT_LEFT,
    DIRECT_LEFT_TOP,
    DIRECT_TOP,
    DIRECT_TOP_RIGHT
};

struct DirectPoint{
    float x ;
    float y;
};
//  x , y  [-1 , 1 ]    z , rz  [-1 , 1]
enum KeyBoardType{
    PC_KeyBoard = 0,
    STICK_KeyBoard
};
class KeyValueTransformt
{
public:
    KeyValueTransformt(QString nameKeyTable , QString defaultKeyBoardPath , QString gameKeyBoardPath );
    ~KeyValueTransformt();
public:
    ////
    /// \brief ConvertXYDirection
    /// \param x
    /// \param y
     /// \param ud   ud  1(keydown)  2(keyup)
    /// \return    根据x,y坐标得出的按键，  可以是组合按键， 全部放在vector中，
    ///
    int ConvertXYDirection(float x , float y , int ud );//方向
    void  ConvertMouse(float &z , float &rz);//鼠标
    ///
    /// \brief KeyDown_C   封装外包按键驱动接口
    /// \param keyName   配置文件中目标键映射的键名称
    /// \param keyValue  配置文件中目标键映射的键值
    /// \return
    ///
    void DirectionAllUp();

    int KeyDown_C(int keyValue);

    int KeyUP_C(int keyValue);
    //鼠标移动
    int MouseMove_C(int x, int y, int code);
    //鼠标抬起
    int MouseUp_C(int x, int y, int code);
    //鼠标按下
    int MouseDown_C(int x, int y, int code);
private:
    ////
    /// \brief GetMapKeyValue   用于普通键盘， 本身就定义了key值的(实体键)
    /// \param orignalKey   为NameValue 中的Value，其实是int格式的字符串
    /// \return
    ///
    QString GetMapKeyValue(QString  orignalKey);
    ////
    /// \brief GetMapKeyValueEx  用于手柄摇杆之类的， 本身没有定义key值的(需要转化为虚拟键)
    /// \return
    ///
    int GetMapKeyValueEx(DirectionKey direction, std::vector<int> &vector);

    ///从加载好的map表中获取正确的映射后的值
    QString GetKeyValueInMap(QString orignalKey);

    ///增加映射表中的
    int SetKeyValueInMap(QString key , QString value);

    void AddKeyValueToThread(Products callback);

    void ReverseBounce(float x , float y);

private:
     std::shared_ptr<DealIniFile> m_nameKeyTable;
     std::shared_ptr<DealIniFile> m_defaultKeyBoardPath;
     std::shared_ptr<DealIniFile> m_gameKeyBoardPath;
     std::map<std::string , std::string> m_keyValueMap;
     DirectionKey m_lastDirect;
     std::shared_ptr<ConsumerKeyBoardThread>  m_consumerKeyboard;

     DirectPoint m_lastDirectPoint;
};


}

#endif // KEYVALUETRANSFORMT_H
