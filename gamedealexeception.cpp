#include "gamedealexeception.h"
#include <string>
namespace GameDealExeceptionSpace{

GameDealExeception::GameDealExeception(std::string errorMsg,int errorCode):logic_error(errorMsg.c_str()) , m_errorCode(std::to_string(errorCode))
{
}


GameDealExeception::~GameDealExeception(){

}

std::string GameDealExeception::GetErrorCode(){
    return m_errorCode;
}

}
