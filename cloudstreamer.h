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
#include <QTimer>
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
    virtual void InterruptCallback(std::string message , int error);
signals:
    void  RecordSignal(QString flagStr , QString logStr);
    void  DisConnect();
    void AccidentalSignal();
protected slots:
    void OnKeyPingTimer();
    void AccidentalTermination();
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
    void StartWebSocket();
    void CloseWebsocket();

    void StartKeyPingTimer();
    void StopKeyPingTImer();

    void StartAccidental();
    void StopAccidental();

    void StartReconnectThread();
    void StopReconnectThread();
private:
    bool m_threadFlag;
    QString m_wsUrl ;
    QString m_loginCommand;
protected:
    std::shared_ptr<KeyValueTransformt> m_keyBoardConfig;
    std::shared_ptr<wsBoostConnect>  m_wsBoostSocket;
    QTimer *m_keyPingTimer;
    QThread   * m_keyPingThread;
    std::shared_ptr<std::thread>m_reconnectThread;
};

enum UI_MODE{
    DEFAULT_MODE= 0 ,
    ONLY_PUSH_STREAMER,
    ONLY_KEY_BOARD
};

QString  WSServiceTransferSignStringEx(QString deviceNo, QString sessionId , QString gameId , QString gamePath);

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
signals:
    void ChangeCloudStreamerStatue(QString statusContent);
    void InputLog(QString flagStr,QString logStr);
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

    void on_comboBox_4_currentIndexChanged(int index);

    void on_game_status_timer();

    void on_changeCloudStreamerStatue(QString statusContent);
    void on_inputLog(QString flagStr,QString logStr);
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
    void ActiveReportGameStatus();

    void ProtectGstLaunch();
    void StartProtectGstLaunch();
    void StopProtectGstLaunch();

    void RecoveryGame();
public:
    void  addLogToEdit(QString flagStr , QString logStr);//添加一条日志到日志
protected:
    QString GetGamePathByID(QString gameId);
    QString GetGameStopByID(QString gameId);
    QString GetValueByGameID(QString gameId , QString keyName);
    bool    GameIsAreadlyRunning(QString gameId);
    bool    KillAreadlyRunningGame(QString gameId);//同步
    bool    StartGameByGameId(QString gameId ,QString startGameParams);
    void    StopWorkThread(std::shared_ptr<std::thread> thread);
    void    StartReportStatusTimer();
    void    StopReportStatusTimer();

    void StartAutoUpdate();
    void CloseAutoUpdate();
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
    QString m_gameId;
    QTimer *m_gameStatusTimer;
    std::shared_ptr<std::thread> m_startGameThread;
    std::shared_ptr<std::thread> m_stopGameThread;
    std::shared_ptr<std::thread> m_signInThread;
    QThread * m_gameStatusThread;
    std::shared_ptr<std::thread>m_gstlaunchProtectThead;
    bool m_gstlaunchProtectTheadFlag;
    std::function<void()> m_pushStreamerFunc;
    std::function<void(int)> m_startGameFunc;
};
#endif // CLOUDSTREAMER_H
