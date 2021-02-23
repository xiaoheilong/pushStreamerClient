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
        void RecordInfo(QString startGameParame ,  int gameStatus);
        void RecordInfo(int gameStatus);
        QString GetGameInfo();
        int GetGameStatus();

    };
}
#endif // RECORDGAMEINFO_H
