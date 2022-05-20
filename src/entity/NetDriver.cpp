#include"NetDriver.h"


NetDriver::NetDriver(const NetDirverCircle& circle,NetDriverSendPolicy s_policy,NetDriverReceivePolicy r_policy):
    m_netInfo(circle),m_sendpolicy(s_policy),m_receivepolicy(r_policy)
{
}

NetDriver::~NetDriver(){
}

void NetDriver::submit(TCPPackageInfo tcpinfo,CallBack call){
    m_sendpolicy(m_netInfo,tcpinfo,call);
}

void NetDriver::receive(TCPPackageInfo tcpinfo,CallBack call){
    m_receivepolicy(m_netInfo,tcpinfo,call);
}
