#include"LinkManager.h"
#include"TCPController.h"
#include"EventLoop.h"
#include"NodeManager.h"
#include"Connection.h"
#include "Base.h"

LinkManager::LinkManager(NodeID id):
    port(5),
    acceptStatus(false),
    RayServiceStatus(false),
    ObjectTableStatus(false)
{
}

LinkManager::~LinkManager(){
}

void LinkManager::addPortLink(Port port,std::shared_ptr<TCPController> tcp){
    m_portToTcpLink[port]=tcp;
}

void LinkManager::connect(Connection connection,ConnectBack call=nullptr){
    //生成一个tcp链接
    //这里的sourceId和destID由上面指定sourcePort也是上面指定
    std::shared_ptr<TCPController> tcplayer=std::make_shared<TCPController>();
    m_portToTcpLink[connection.m_sourcePort]=tcplayer;

    Message message;
    enum connect msg = SYN;
    message.buff = (void *)(msg);
    message.size = 256;
    message.bufflength = 4;
	
    //将包放入对应的tcplayer
    //一个事件放入到
    if(call!=nullptr){
        tcplayer->setconnectionCallBack(call);
    }
	
    tcplayer->sendMessage(TCPPackageInfo(connection,message),ACCEPT);
}

Connection LinkManager::accept(ConnectBack call,int port=0){
    std::shared_ptr<TCPController> tcplayer=getTCPLayer(port);
    if(tcplayer==nullptr){
        return Connection();
    }
	
    //std::cout << tcplayer-> << std::endl;
    if(tcplayer->empty()){
        tcplayer->setCallback(std::bind(&LinkManager::accept,this,call,port));
        return Connection();
    }
	
    TCPPackageInfo package=tcplayer->getFromReceiveList();
    if(call!=nullptr){
        EventLoop::getInstance().addCallBack(std::bind(call,package.first));
    }
    return package.first;
}

void LinkManager::ReplyConnect(Connection& conn){
    Port port=CreatePort();

    conn.m_sourcePort=port;
    
    std::shared_ptr<TCPController> tcplayer=std::make_shared<TCPController>();
    m_portToTcpLink[conn.m_sourcePort]=tcplayer;
    Message message;
    enum connect msg=SYN_ACK;
    message.buff=(void*)(msg);
    message.bufflength=8;
    message.size=256;
    tcplayer->sendMessage(TCPPackageInfo(conn,message),CONNECT);
}

void LinkManager::setReplyConnect(ConnectAtferBack call){
    //首先找到port=0的端口
    std::shared_ptr<TCPController> tcplayer=getTCPLayer(0);
    tcplayer->setReplyCallBack(call);
}

void LinkManager::listen(){
    if(acceptStatus){
        return;
    }
	
    std::shared_ptr<TCPController> tcplayer = std::make_shared<TCPController>();
    m_portToTcpLink[GlobalInfomationPort] = tcplayer;
    acceptStatus=true;
    tcplayer->setReplyCallBack(std::bind(&LinkManager::ReplyConnect,this,std::placeholders::_1));
}

void LinkManager::listenRayService(){
    if(RayServiceStatus){
        return;
    }
    std::shared_ptr<TCPController> tcplayer=std::make_shared<TCPController>();
    m_portToTcpLink[RayServicePort]=tcplayer;
    RayServiceStatus=true;
    tcplayer->setReplyCallBack(std::bind(&LinkManager::ReplyConnect,this,std::placeholders::_1));
}

void LinkManager::listenObjectTable(){
    if(ObjectTableStatus){
        return;
    }
    std::shared_ptr<TCPController> tcplayer=std::make_shared<TCPController>();
    m_portToTcpLink[ObjectTablePort]=tcplayer;
    ObjectTableStatus=true;
    tcplayer->setReplyCallBack(std::bind(&LinkManager::ReplyConnect,this,std::placeholders::_1));
}

/*没有去考虑写的时候tcp链接没建立的情况，直接模拟的write的情况*/
bool LinkManager::write(Connection connection,Message message,CallBack call=nullptr){
    std::shared_ptr<TCPController> tcplayer=m_portToTcpLink[connection.m_sourcePort];
    tcplayer->sendMessage(TCPPackageInfo(connection,message),RECEIVE);
    
    if(call!=nullptr){
        EventLoop::getInstance().addCallBack(call);
    }
    return true;
}

bool LinkManager::read(Connection connection,ReadBack call){
    //先通过connection去寻找tcplayer
    
    std::shared_ptr<TCPController> tcplayer = m_portToTcpLink[connection.m_sourcePort];

	if(tcplayer==nullptr){
        m_portToTcpLink[connection.m_sourcePort].get();//这儿这个get()函数在哪儿定义的？@wanghs
        std::cout << "tcplayer==nullptr" << std::endl;
    }
	
    if(tcplayer->empty()){
        tcplayer->setCallback(std::bind(&LinkManager::read,this,connection,call));
        return true;
    }
	
    Message message = tcplayer->getMessageFromReciveList(connection);

    if(call!=nullptr){
        EventLoop::getInstance().addCallBack(std::bind(call,message));
    }
    return true;
}

bool LinkManager::close(Connection connection){
    //将消息传递给对面的用户
    Message msg;
    msg.size=256;
    msg.bufflength=5;
    msg.buff=(void *)(TIME_OUT);
    std::shared_ptr<TCPController> tcplayer=m_portToTcpLink[connection.m_sourcePort];
    tcplayer->sendMessage(TCPPackageInfo(connection,msg),CLOSE);
    //将自身的删除
    return deleteConnect(connection);
}

bool LinkManager::deleteConnect(Connection conn){
    if(m_portToTcpLink.find(conn.m_sourcePort)==m_portToTcpLink.end()){
        return true;
    }
    m_portToTcpLink.erase(conn.m_sourcePort);
    return true;
}

std::shared_ptr<Node> LinkManager::getDataSource(NodeID id){
    return NodeManager::getIntance().searchNode(id);
}

Port LinkManager::CreatePort(){
    return port++;
}

std::shared_ptr<TCPController> LinkManager::getTCPLayer(Port port){
    if(m_portToTcpLink.find(port)==m_portToTcpLink.end()){
        return nullptr;
    }
    return m_portToTcpLink.at(port);
}
