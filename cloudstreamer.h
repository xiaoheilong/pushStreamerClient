#ifndef CLOUDSTREAMER_H
#define CLOUDSTREAMER_H
#include "cloudgameserviceiterator.h"
#include "dealinifile.h"
//#include "cloudgaestreamdeal.h"
#include  "keyvaluetransformt.h"
#include  "websocketConnection.h"
#include "websocketConnection.h"
using namespace CloudGameServiceIteratorSpace;
#include <QMainWindow>
#include <QString>
#include <QThread>
#include <memory>
#include <map>
namespace Ui {
class CloudStreamer;
}
using namespace DealIniFileSpace;
using namespace KeyValueTransformtNamespace;
using namespace WebSocketNamsSpace;
//typename KeyValueTransformtNamespace::KeyValueTransformt  KeyValueTransformt;
class CloudStreamer;
class KeyBoardThread: public QThread, public OutterInterfaceConnection
{
    Q_OBJECT
public:
    KeyBoardThread( QObject * parent = nullptr);
    ~KeyBoardThread();
    typedef WebSocketNamsSpace::WsAppConnection wsBoostConnect;
public:
    virtual void ConnectedCallback(std::string msg , int error);
    virtual void DisconnectedCallback(std::string msg , int error);
    virtual void MessageCallback(std::string msg , int error);
    virtual void FailureCallback(std::string msg , int error);
    virtual void InterruptCallback(std::string msg , int error);
signals:
    void  RecordSignal(QString flagStr , QString logStr);
    void  DisConnect();
protected:
    virtual void run();

public:
    bool start(QString wsUrl , QString loginCommand);
    bool stop();
    ////
    /// \brief SetKeyBoardModel
    /// \param keyboardConfigIni  is the keyBoardIniConfig ini
    ///
    void SetKeyBoardModel(QString nameKeyTablePath , QString defaultKeyBoardPath , QString gameKeyBoardPath);
private:
    bool m_threadFlag;
    QString m_wsUrl ;
    QString m_loginCommand;
protected:
    std::shared_ptr<KeyValueTransformt> m_keyBoardConfig;
    std::shared_ptr<wsBoostConnect>  m_wsBoostSocket;
};

enum UI_MODE{
    DEFAULT_MODE= 0 ,
    ONLY_PUSH_STREAMER,
    ONLY_KEY_BOARD
};

class CloudStreamer : public QMainWindow ,  public std::enable_shared_from_this<CloudStreamer>
{
    Q_OBJECT
public:
    explicit CloudStreamer(QWidget *parent = 0);
    ~CloudStreamer();
//signals:
//      void StartGameSignal(QString gamePath , QString params); // gamePath is  absolutely path
//      void StopGame(QString gamePath);// gamePath is  absolutely path
//      void Download(QString downloadUrl , QString savePath);//downloadUrl is file url of goal file , savePath is  absolutely path
//      void PushStreamSignal(QString streamParams);  //streamParams is format like json string
//      void CloseStreamSignal(QString streamParams);//streamParams is format like json string
    friend class KeyBoardThread;
protected:
    typedef CloudGameServiceIteratorSpace::CloudGameServiceIterator  CloudGameServiceIterator;
public:
    void BindServiceIterator(std::shared_ptr<CloudGameServiceIterator>  iterator);
    std::shared_ptr<CloudStreamer> GetPtr();

    virtual void closeEvent ( QCloseEvent * event );
private slots:

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    //void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_clicked();

    void on_pushButton_8_clicked();
    ////////////////// message slots from wsServer 
    void RecordSignalCallBack(QString flagStr , QString logStr);
    void ParseMessageCallback(QString data);
     //void DllLogCallback(const char* logType, const char *  logMsg);

    void on_comboBox_4_currentIndexChanged(int index);

private:
    void  install_Driver();
    void  uninstall_Driver();


    void ConnectCallback(QString url, QString data);   // url goal ws server  connect callback  data is some params
    void DisconnectCallback(QString url, QString data); //url goal ws server disconnect callback data is some params
    void StartGameCallback(QString data); // gamePath is  absolutely path
    void StopGameCallback(QString data);// gamePath is  absolutely path
    void DownloadCallback(QString downloadUrl , QString savePath);//downloadUrl is file url of goal file , savePath is  absolutely path
    void PushStreamCallback(QString streamParams);  //streamParams is format like json string
    void CloseStreamCallback(QString streamParams);//streamParams is format like json string
    void SignInCloudGameCallback(QString paramData);
    void SetUIModel(UI_MODE model);
    void SignInWsService();
public:
    void  addLogToEdit(QString flagStr , QString logStr);//添加一条日志到日志
protected:
    QString GetGamePathByID(QString gameId);
    QString GetValueByGameID(QString gameId , QString keyName);
private:
    Ui::CloudStreamer *ui;
    /////////
    bool m_isInstallDriver;
    bool m_isPushStream;
    bool m_isOpenKeyBoard;
    bool m_isWin32Init;
    bool m_keyBoardthreadFlag;
   // QString  m_str;
    std::shared_ptr<KeyBoardThread>  m_keyBoardThread;
    QString m_gamePath;
    QString m_fileSavePath;
    std::shared_ptr<CloudGameServiceIterator> m_cloudGameServiceIterator;
    std::shared_ptr<DealIniFile>  m_iniParse;
    UI_MODE m_mode;
    QString m_sessionId;
    QString m_deviceNo;
    QString m_controlUrl;
    FILE * m_file;
};
#endif // CLOUDSTREAMER_H
