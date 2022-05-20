#ifndef COMPUTATIONTCPLINKPOLICY_H
#define COMPUTATIONTCPLINKPOLICY_H

#define RR 0
#define FIFO 1

#include"TCPController.h"
#include"NetDirverCircle.h"
#include<list>

class SendRRNetDriverPolicy{
    typedef std::pair<std::pair<TCPPackageInfo,CallBack>,unsigned int> Package;
public:
    SendRRNetDriverPolicy(unsigned int);
    ~SendRRNetDriverPolicy();
    void operator()(NetDirverCircle&,TCPPackageInfo,CallBack);
    void send(NetDirverCircle&);
private:
    std::list<Package> m_sendList;
    //void remove();
    void push(TCPPackageInfo,CallBack);
    void addToEventLoop(Time,CallBack);
    unsigned int m_dataSlience;
    bool m_status;
    bool m_nextSendStatus;

    std::list<Package>::iterator m_nextSend;
};

class ReceiveRRNetDriverPolicy{
    typedef std::pair<std::pair<TCPPackageInfo,CallBack>,unsigned int> Package;
public:
    ReceiveRRNetDriverPolicy(unsigned int);
    ~ReceiveRRNetDriverPolicy();
    void push(TCPPackageInfo,CallBack);
    void operator()(NetDirverCircle&,TCPPackageInfo,CallBack);
private:
    void addToEventLoop(Time,CallBack);
    void receive(NetDirverCircle&);
private:
    std::list<Package> m_receiveList;
    //void remove();
    unsigned int m_dataSlience;
    bool m_status;
    bool m_nextReceiveStatus;

    std::list<Package>::iterator m_nextReceive;
};


#endif