#ifndef CLOUDSTREAMER_H
#define CLOUDSTREAMER_H
#include "cloudgameserviceiterator.h"
#include "dealinifile.h"
//#include "cloudgaestreamdeal.h"
#include  "keyvaluetransformt.h"
#include  "websocketConnection.h"
#include "websocketConnection.h"
#include "json/json.h"
using namespace CloudGameServiceIteratorSpace;
#include <QMainWindow>
#include <QString>
#include <QThread>
#include <memory>
#include <map>
#include <QTimer>
#include <QtWidgets>
#include <string>
namespace Ui {
class CloudStreamer;
}

    typedef struct _PushStreamerParams{
        void Set(QString serverUrl , QString domain , QString roomId , QString framerate, QString bitrate ,QString deadline , QString cpuused ,QString x , QString y
                      ,QString mode, QString capmode , QString vol){
            m_serverUrl= serverUrl.toStdString();
            m_domain = domain.toStdString();
            m_roomId = roomId.toStdString();
            m_framerate =framerate.toStdString();
            m_bitrate = bitrate.toStdString();
            m_deadline = deadline.toStdString();
            m_cpuused = cpuused.toStdString();
            m_x = x.toStdString();
            m_y = y.toStdString();
            m_mode = mode.toStdString();
            m_capmode = capmode.toStdString();
            m_vol = vol.toStdString();
        }
        std::string LogStr(){
            return  m_serverUrl +"," + m_domain +"," + m_roomId +"," + m_framerate + "," + m_bitrate +"," + m_deadline + "," + m_cpuused + "," + m_x + "," + \
                     m_y + "," + m_mode + "," + m_capmode + "," + m_vol;
        }
      public:
          std::string m_serverUrl;
          std::string m_domain;
          std::string m_roomId;
          std::string m_framerate;
          std::string m_bitrate;
          std::string m_deadline;
          std::string m_cpuused;
          std::string m_x;
          std::string m_y;
          std::string m_mode;
          std::string m_capmode;
          std::string m_vol;
    }PushStreamerParams;

    Q_DECLARE_METATYPE(PushStreamerParams)

    typedef struct _StartGameParams{
        std::string m_gameId;
        std::string m_startGameParams;
        std::string m_data;
        int m_force;
        void SetValue(QString gameId , QString startGameParams , QString data , int force){
            m_gameId = gameId.toStdString();
            m_startGameParams = startGameParams.toStdString();
            m_data = data.toStdString();
            m_force = force;
        }
    }StartGameParams;
    Q_DECLARE_METATYPE(StartGameParams)
using namespace DealIniFileSpace;
using namespace KeyValueTransformtNamespace;
using namespace WebSocketNamsSpace;
//typename KeyValueTransformtNamespace::KeyValueTransformt  KeyValueTransformt;
int isProcessRunning(QString processName);
std::wstring s2ws(const std::string &s);
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

    void SetGamePid(int gamePid);
    int GetGamePid();
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

    void SetDeviceNumber(QString deviceNum);
    QString GetDeviceNumber();
private:
    void StartWebSocket();
    void CloseWebsocket();

    void StartKeyPingTimer();
    void StopKeyPingTImer();


    void StartAccidental();
    void StopAccidental();

    void StartReconnectThread();
    void StopReconnectThread();

    bool IsLeftKeyboardNumberLetter(int & keyValue);
    void DealTheFullKeyboardModel(Json::Value  & root);//root is the pointer of Json::Value , Do it first, maybe optimize in future
    void DealTheNormalKeyboardModel(Json::Value  &root);
    void DealTheJoystick(Json::Value  &root);   

    bool IsGameWindowActive();
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
    std::shared_ptr<DealIniFile>  m_iniParse1;
    std::shared_ptr<DealIniFile>  m_iniParse2;
    bool m_isUpperKey;
    QString m_deviceNumberl;
    int m_gamePid;
};

enum UI_MODE{
    DEFAULT_MODE= 0 ,
    ONLY_PUSH_STREAMER,
    ONLY_KEY_BOARD
};

QString  WSServiceTransferSignStringEx(QString deviceNo, QString sessionId , QString gameId , QString gamePath);

enum StartGameModel{
    NORMAL_MODEL = 0,
    ONLY_PUSHSTREAMR
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
signals:
    void ChangeCloudStreamerStatue(QString statusContent);
    void InputLog(QString flagStr,QString logStr);
protected:
    typedef CloudGameServiceIteratorSpace::CloudGameServiceIterator  CloudGameServiceIterator;
    //typedef CloudStreamerSpace::PushStreamerParams  PushStreamerParams;
    //typedef CloudStreamerSpace::StartGameParams   StartGameParams;
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

    void GameStatusCallback();

    void on_changeCloudStreamerStatue(QString statusContent);
    void on_inputLog(QString flagStr,QString logStr);
private slots://UI slot
    int OnSystemTrayClicked(QSystemTrayIcon::ActivationReason reason);
    int OnExit();
protected://windows event
     void changeEvent(QEvent * event);
private:
    void  install_Driver();
    void  uninstall_Driver();


    void ConnectCallback(QString url, QString data);   // url goal ws server  connect callback  data is some params
    void DisconnectCallback(QString url, QString data); //url goal ws server disconnect callback data is some params
    void StartGameCallback(QString data , StartGameModel model = NORMAL_MODEL); // gamePath is  absolutely path
    void StopGameCallback(QString data);// gamePath is  absolutely path
    void DownloadCallback(QString downloadUrl , QString savePath);//downloadUrl is file url of goal file , savePath is  absolutely path
    void PushStreamCallback(QString streamParams);  //streamParams is format like json string
    void CloseStreamCallback(QString streamParams);//streamParams is format like json string
    void SignInCloudGameCallback(QString paramData);
    void ChangeResolutionCallback(QString paramData);
    void MessageFeedBack(QString msgType ,  QString resultCode , QString resultMsg);

    QString ModifiyStartParams(QString framerate, QString bitrate,QString deadline);
    void SetUIModel(UI_MODE model);
    void SignInWsService();
    void ActiveReportGameStatus();

    void ProtectGstLaunch();
    void StartProtectGstLaunch();
    void StopProtectGstLaunch();

    void RecoveryGame();

    void QuitForce(bool value);
    void KillGameByName(QString gameName);

    void ClearFunctionParams();

    int  PushStreamerAction(QString serverUrl , QString domain , QString roomId , QString framerate, QString bitrate ,QString deadline , QString cpuused ,QString x , QString y
                            ,QString mode, QString capmode , QString vol);
    int  StartGameAction(QString gameId , QString startGameParams , QString data , int force);

    int  StartKeyboardAction(QString gameId , QString controlUrl , QString keyboardLoginParams);

    void LastGaspGoalPushStreamer();
public:
    void  addLogToEdit(QString flagStr , QString logStr);//添加一条日志到日志
protected:
    QString GetGamePathByID(QString gameId);
    QString GetGameStopByID(QString gameId);
    QString GetValueByGameID(QString gameId , QString keyName);
    void ForeachKillExe(QString gameId);
    bool    GameIsAreadlyRunning(QString gameId);
    bool    KillAreadlyRunningGame(QString gameId);//同步
    bool    StartGameByGameId(QString gameId ,QString startGameParams);
    void    StopWorkThread(std::shared_ptr<std::thread> thread);
    void    StartReportStatusTimer();
    void    StopReportStatusTimer();

    void StartAutoUpdate();
    void CloseAutoUpdate();

    bool isUpdate();

    int InitSystemTray();
    int UInitSystemTray();

    void ExecuteOutterScript(QString  path, const QStringList &params);
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
    std::shared_ptr<DealIniFile>  m_StreamConfigIniParse;
    std::shared_ptr<DealIniFile>  m_CloudGameConfigIniParse;
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
    std::shared_ptr<std::thread> m_changeResolution;
    QThread * m_gameStatusThread;
    std::shared_ptr<std::thread>m_gstlaunchProtectThead;
    bool m_gstlaunchProtectTheadFlag;
    std::function<void()> m_pushStreamerFunc;
    std::function<void(int)> m_startGameFunc;
    std::mutex m_gstLaunchMutex;
    QSystemTrayIcon *m_systray;
    PushStreamerParams m_pushStreamerParams;
    StartGameParams  m_startGameParams;
    int m_gamePid;
    std::mutex m_gameMutex;
};
#endif // CLOUDSTREAMER_H
