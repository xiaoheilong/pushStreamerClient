#ifndef CLOUDGAMESERVICEITERATOR_H
#define CLOUDGAMESERVICEITERATOR_H

#include <QObject>
namespace CloudGameServiceIteratorSpace{
    class CloudGameServiceIterator : public QObject
    {
        Q_OBJECT
    public:
        explicit CloudGameServiceIterator(QObject *parent = nullptr);
    signals:
        void Connect(QString url , QString data);
        void Disconnect(QString url, QString data);
        void StartGame(QString gamePath , QString gameParame);  //开启本地游戏
        void StopGame(QString gamePath);   //关闭本地游戏
        void Download(QString downloadUrl , QString saveLocalPath);    // 下载文件
        void PushStream(QString paramData);   // format is json string   推流
        void CloseStream(QString parmData);   // format is json string   关流
        void SignInCloudGame(QString paramData); //signin game ws server
        void ParseMessage(QString paramData);
        void Send(QString data);
    public slots:

    };
}
#endif // CLOUDGAMESERVICEITERATOR_H
