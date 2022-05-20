#include"NetDriverPolicy.h"
#include"EventLoop.h"
#include"NetDriverManager.h"
#include"NetDriver.h"

#include <algorithm>

const unsigned int maxSendSize=1024;
const unsigned int minSendSize=128;



//通过时间片轮转进行发送
SendRRNetDriverPolicy::SendRRNetDriverPolicy(unsigned int dataSlience):
    m_dataSlience(dataSlience),
    m_status(true),
    m_nextSendStatus(true)
{
    //m_nextSend=m_sendList.end();
}

SendRRNetDriverPolicy::~SendRRNetDriverPolicy(){
}

void SendRRNetDriverPolicy::push(TCPPackageInfo tcpPackage,CallBack call){
    int size;
	
    if(tcpPackage.second.size<minSendSize){
        size=minSendSize;
    }
    else{
        size=tcpPackage.second.size;
    }
    m_sendList.push_back(Package(std::pair<TCPPackageInfo,CallBack>(tcpPackage,call),size));
}

void SendRRNetDriverPolicy::addToEventLoop(Time time,CallBack call){
    EventLoop::getInstance().callAfter(time,call);
}

void SendRRNetDriverPolicy::send(NetDirverCircle& circle){
    //这里再t0和t1+t0帮定2个回调函数
    //一个是自己的回调函数，一个是将package绑定给对方的回调函数
    if(m_sendList.empty()){
        m_status=true;
        return;
    }
    else{
        while (circle.m_sendcircleSize>0)
        {
            if(m_sendList.empty()){
                break;
            }
			
            if(m_nextSend==m_sendList.end()){
                m_nextSend=m_sendList.begin();
            }
			
            if(m_nextSendStatus){
                m_nextSendStatus=false;
                m_nextSend=m_sendList.begin();
            }
            
            unsigned int curRoundSize=std::min({m_dataSlience,m_nextSend->second,circle.m_sendcircleSize});
            m_nextSend->second-=curRoundSize;
            circle.m_sendcircleSize-=curRoundSize;
			
            if(m_nextSend->second==0){
                std::list<Package>::iterator temp = m_nextSend;
                auto netDriver=NetDriverManager::getIntance().searchNetDriver(m_nextSend->first.first.first.m_destID);

				addToEventLoop(circle.m_senddelay+circle.m_internetdelay,
                    std::bind(&NetDriver::receive,netDriver,m_nextSend->first.first,m_nextSend->first.second));
				
                m_nextSend++;
                m_sendList.erase(temp);
            }
			
            if(circle.m_sendcircleSize==0){
                break;
            }
        }
		
        addToEventLoop(circle.m_senddelay,[&circle,this]()mutable{
            circle.m_sendcircleSize=circle.m_senddelay*circle.m_upBandWidth;
            this->send(circle);
        });
    }
}

void SendRRNetDriverPolicy::operator()(NetDirverCircle& circle,TCPPackageInfo package,CallBack call){
    //将对应的package和回调函数压入
    push(package,call);
    //压入之后进行发送
    if(m_status){
        m_status=false;
        send(circle);
    }
}


ReceiveRRNetDriverPolicy::ReceiveRRNetDriverPolicy(unsigned int dataSlience):
    m_dataSlience(dataSlience),
    m_status(true),
    m_nextReceiveStatus(true)
{
    //m_nextReceive=m_receiveList.end();
}

ReceiveRRNetDriverPolicy::~ReceiveRRNetDriverPolicy(){
}

void ReceiveRRNetDriverPolicy::push(TCPPackageInfo tcpPackage,CallBack call){
    int size=tcpPackage.second.size;
    m_receiveList.push_back(Package(std::pair<TCPPackageInfo,CallBack>(tcpPackage,call),size));
}

void ReceiveRRNetDriverPolicy::addToEventLoop(Time time,CallBack call){
    EventLoop::getInstance().callAfter(time,call);
}

void ReceiveRRNetDriverPolicy::receive(NetDirverCircle& circle){
    if(m_receiveList.empty()){
        m_status=true;
        return;
    }
    else{
        while(circle.m_recivecircleSize>0){
            if(m_receiveList.empty()){
                break;
            }
            if(m_receiveList.end()==m_nextReceive){
                m_nextReceive=m_receiveList.begin();
            }
            if(m_nextReceiveStatus){
                m_nextReceiveStatus=false;
                m_nextReceive=m_receiveList.begin();
            }
            unsigned int curRoundSize=std::min({m_dataSlience,m_nextReceive->second,circle.m_recivecircleSize});
            circle.m_recivecircleSize-=curRoundSize;
            m_nextReceive->second-=curRoundSize;
            if(m_nextReceive->second==0){
                auto temp=m_nextReceive;
                addToEventLoop(circle.m_senddelay,m_nextReceive->first.second);
                m_nextReceive++;
                m_receiveList.erase(temp);
            }
            if(circle.m_recivecircleSize==0){
                break;
            }
        }
        addToEventLoop(circle.m_senddelay,[&circle,this]()mutable{
            circle.m_recivecircleSize=circle.m_senddelay*circle.m_downBandWidth;
            this->receive(circle);
        });
    }
}

void ReceiveRRNetDriverPolicy::operator()(NetDirverCircle& circle,TCPPackageInfo package,CallBack call){
    push(package,call);
    if(m_status){
        m_status=false;
        receive(circle);
    }
}

