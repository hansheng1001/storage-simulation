#ifndef NETDRIVER_H
#define NETDRIVER_H
#include"TCPController.h"
#include"NetDirverCircle.h"

#include<list>
#include<functional>

using NetDriverReceivePolicy=std::function<void(NetDirverCircle&,TCPPackageInfo,CallBack)>;
using NetDriverSendPolicy=std::function<void(NetDirverCircle&,TCPPackageInfo,CallBack)>;

class NetDriver
{

private:
    /* data */
    NetDirverCircle           m_netInfo;
    NetDriverSendPolicy       m_sendpolicy; 
    NetDriverReceivePolicy    m_receivepolicy;
    
public:
    NetDriver(const NetDirverCircle&,NetDriverSendPolicy,NetDriverReceivePolicy);
    ~NetDriver();
    void submit(TCPPackageInfo,CallBack);
    void receive(TCPPackageInfo tcpinfo,CallBack call);
    void setNetInfo(const NetDirverCircle& circle){
        m_netInfo=circle;
    }

    NetDirverCircle getNetInfo()
    {
        return m_netInfo;
    }
};

#endif