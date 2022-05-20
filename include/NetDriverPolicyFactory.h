#ifndef COMPUTATIONTCPLINKFACTORY_H
#define COMPUTATIONTCPLINKFACTORY_H

#include"Node.h"
#include"NetDirverCircle.h"
#include"NetDriverPolicy.h"
#include<memory>

class NetDriver;
std::shared_ptr<NetDriver> CreateNetDriver(NetDirverCircle info,unsigned long time){
    return std::make_shared<NetDriver>(info,SendRRNetDriverPolicy(time),ReceiveRRNetDriverPolicy(time));
}



#endif