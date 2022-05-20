#include"TCPController.h"
#include"Event.h"
#include"EventLoop.h"
#include"Callbacks.h"
#include"Node.h"
#include"NodeManager.h"
#include"ManagerLinkUnOrderMap.h"
#include"LinkManager.h"
#include"Connection.h"
#include"NetDriverManager.h"
#include"NetDriver.h"

#include<memory>

TCPController::~TCPController(){
}

TCPController::TCPController():
    m_CallBack(nullptr)
{
}

void TCPController::sendMessage(TCPPackageInfo package,ReceiveCategory category){
    std::shared_ptr<Node> node = NodeManager::getIntance().searchNode(package.first.m_destID);

	/*每个Node对应1个Link，一个Link中又可能对应多个TCP链接@wanghs*/
    std::shared_ptr<LinkManager> link = ManagerLinkUnOrderMap::getIntance().searchLink(package.first.m_destID);

    if(link==nullptr){
        std::cout << "link == nullptr" << std::endl;
    }
	
    //查找对应的TCP链接
    /*没找到连接时应该直接退出的@wanghs*/
    std::shared_ptr<TCPController> tcpLayer = link->getTCPLayer(package.first.m_destPort);
    if(tcpLayer==nullptr){
        std::cout << "tcplayer == nullptr" << std::endl;
    }
	
    auto netDriver=NetDriverManager::getIntance().searchNetDriver(package.first.m_sourceID);
    if(netDriver==nullptr){
        std::cout << "netDrive ==nullptr" << std::endl;
    }

	/*下面的消息类型有点混乱@wanghs*/
    switch(category){
        case ACCEPT:/*TCP客服端请求建立连接是发送的消息，可以改为connect类型@wanghs*/
            netDriver->submit(package,std::bind(&TCPController::receiveAcceptMessage,tcpLayer,package.first,package.second));
            break;
        case CONNECT:/*TCP服务器端收到connect消息后回的消息，可以改为reply类型@wanghs*/
            netDriver->submit(package,std::bind(&TCPController::receiveConnectionMessage,tcpLayer,package.first,package.second));
            break;
        case RECEIVE:
            netDriver->submit(package,std::bind(&TCPController::receiveMessage,tcpLayer,package.first,package.second));
            break;
        case CLOSE:
            netDriver->submit(package,std::bind(&TCPController::receiveCloseMessage,tcpLayer,package.first,package.second));
            break;
        default:
            break;
    }
}

void TCPController::setCallback(CallBack call){
    m_CallBack=std::move(call);
}

void TCPController::setconnectionCallBack(ConnectBack call){
    m_connectionCallBack=std::move(call);
}

void TCPController::setReplyCallBack(ConnectAtferBack call){
    m_replyConnection=std::move(call);
}

//这里要把m_CallBack设置为Message
void TCPController::receiveAcceptMessage(Connection connection,Message message){
    connection.swap();
    m_replyConnection(connection);
    //首先先将connection和message放入对应的TCP链接connection.destPort
    m_recive.push_back(TCPPackageInfo(connection,message));

	
    //这里需不需要通知节点有消息到了
    //由这个函数去触发刚刚的回调
    if(m_CallBack!=nullptr){
        EventLoop::getInstance().addCallBack(m_CallBack);
    }
}

void TCPController::receiveConnectionMessage(Connection connection,Message message){
    connection.swap();
    if(m_connectionCallBack!=nullptr){
        EventLoop::getInstance().addCallBack(std::bind(m_connectionCallBack,connection));
        m_connectionCallBack=nullptr;/*马上置为Null，是否存在问题？复制拷贝应该没有问题@wanghs*/
    }
}

void TCPController::receiveMessage(Connection connection,Message message){
    connection.swap();
    m_recive.push_back(TCPPackageInfo(connection,message));

    if(m_CallBack!=nullptr){
        EventLoop::getInstance().addCallBack(m_CallBack);
        if(m_CallBack==nullptr){
            std::cout << "m_CallBack == nullptr" << std::endl;
        }
    }
}

void TCPController::receiveCloseMessage(Connection connection,Message message){
    connection.swap();
    std::shared_ptr<LinkManager> link=ManagerLinkUnOrderMap::getIntance().searchLink(connection.m_sourceID);
    link->deleteConnect(connection);
}

Message TCPController::getMessageFromReciveList(Connection connection){
    auto begin=m_recive.begin();
    auto end=m_recive.end();
    for(;begin!=end;begin++){
        if(begin->first==connection){
            break;
        }
    }
    if(begin==end){
        return Message();
    }
    Message msg=begin->second;
    m_recive.erase(begin);
    return msg;
}

// Message TCPController::getMessageFromReciveList(Connection connection){
//     auto begin=m_recive.begin();
//     auto end=m_recive.end();
//     for(;begin!=end;begin++){
//         if(begin->first==connection){
//             return begin->second;
//         }
//     }
//     return Message();//这里的修改
// }

TCPPackageInfo TCPController::getFromReceiveList(){
    TCPPackageInfo packge=m_recive.front();
    m_recive.pop_front();
    return packge;
}

bool TCPController::empty(){
    return m_recive.empty();
}



