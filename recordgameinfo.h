#ifndef RECORDGAMEINFO_H
#define RECORDGAMEINFO_H
#include "dealinifile.h"
#include <QObject>
#include <mutex>
#include <memory>
namespace RecordGameInfoSpace{
    using namespace DealIniFileSpace;
    class RecordGameInfo
    {
    public:
       // explicit RecordGameInfo() = delete;
        ~RecordGameInfo();
        static RecordGameInfo * GetInstance();
        static void ReleaseInstance();
    private:
        explicit RecordGameInfo();
    public:
        void RecordInfo(QString startGameParame , QString gameId, int gameStatus);
        void RecordInfo(int gameStatus);
        void RecordInfo(QString gameId,int gameStatus);
        void RecordInfo(QString startGameParame);
        QString GetGameInfo();
        int GetGameStatus();
        QString GetGameId();
        bool GetIsUpdate();
    private:
        void InitDealIniFile();
    protected:
        static RecordGameInfo * m_instance;
        std::shared_ptr<DealIniFile> m_dealIniFile;
    };
}
#endif // RECORDGAMEINFO_H
