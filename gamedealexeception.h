#ifndef GAMEDEALEXECEPTION_H
#define GAMEDEALEXECEPTION_H
#include <stdexcept>
namespace GameDealExeceptionSpace{

class GameDealExeception:public std::logic_error
{
public:
    explicit GameDealExeception(std::string errorMsg , int errorCode);
    GameDealExeception() = delete;
    ~GameDealExeception();
public:
    std::string GetErrorCode();
private:
    std::string m_errorCode;
};


}
#endif // GAMEDEALEXECEPTION_H
