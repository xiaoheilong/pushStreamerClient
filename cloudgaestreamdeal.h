#ifndef CLOUDGAESTREAMDEAL_H
#define CLOUDGAESTREAMDEAL_H
#include<string>
#include<iostream>
namespace CloudGaeStreamDealSpace{
class CloudGaeStreamDeal
{
public:
    CloudGaeStreamDeal();
public:
    int StartGame(std::string data);//json str
    int StopGame(std::string data); //json str
};

}
#endif // CLOUDGAESTREAMDEAL_H
