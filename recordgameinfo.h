#ifndef RECORDGAMEINFO_H
#define RECORDGAMEINFO_H

#include <QObject>
namespace RecordGameInfoSpace{

    class RecordGameInfo
    {
    public:
        explicit RecordGameInfo();
        ~RecordGameInfo();
    public:
        void RecordInfo(QString startGameParame , QString gameId, int gameStatus);
        void RecordInfo(int gameStatus);
        void RecordInfo(QString gameId,int gameStatus);
        void RecordInfo(QString startGameParame);
        QString GetGameInfo();
        int GetGameStatus();
        QString GetGameId();

    };
}
#endif // RECORDGAMEINFO_H
