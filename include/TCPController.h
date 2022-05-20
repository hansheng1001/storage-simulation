#ifndef TCPCONTROLLER_H
#define TCPCONTROLLER_H

#include"Callbacks.h"

#include<memory>
#include<list>
#include<mutex>
#include<map>
#include<condition_variable>
#include <unordered_map>
#include"Node.h"
#include"Connection.h"

class Event;
class EventLoop;
class NodeManager;
class Node;
class LinkManager;

typedef std::pair<Connection,Message> TCPPackageInfo;

typedef std::pair<Time,TCPPackageInfo> TCPPackage;

class TCPController{

private:
    TCPPackageInfo m_send;

    // std::list<TCPPackageInfo> m_sendList;
    // std::mutex m_sendMutex;
    // bool m_sendStatus;

    std::list<TCPPackageInfo> m_recive;

    //
    /*这个函数是当tcp收到message的回调函数，一般是
    *调LinkManager中的read没读到数据后，置的read本身为其回调函数@wanghs
    */
    CallBack        m_CallBack;
	
    /*这是TCP客户端时，LinkManager调用connect函数时置的回调函数，
    *它一般是链接建立后,收到tcp服务器端的reply消息后回调执行。
    */
    ConnectBack     m_connectionCallBack;
    
    /*这是作为TCP服务端时，收到connect消息后触发的回调函数，是
    LinkManager中的Listen函数注册的创建链接的函数@wanghs*/
    ConnectAtferBack m_replyConnection;

public:
    TCPController();
    ~TCPController();
	
    void sendMessage(TCPPackageInfo package, ReceiveCategory);
    //void sendConnectionMessage(TCPPackageInfo package);

    // void sendMessageList();
    // void addMessageToSendList(TCPPackageInfo);
    void setCallback(CallBack call);
    void setconnectionCallBack(ConnectBack call);
    void setReplyCallBack(ConnectAtferBack);

    void receiveAcceptMessage(Connection,Message);//1
    void receiveConnectionMessage(Connection,Message);//2
    void receiveMessage(Connection,Message);//3
    void receiveCloseMessage(Connection,Message);

    Message getMessageFromReciveList(Connection);
    TCPPackageInfo getFromReceiveList();
    //这个是给上面判断m_recive是不是空
    bool empty();
    
    
    int  size(){
        return 0;
    }
    // std::list<TCPPackageInfo>::iterator begin();
    // std::list<TCPPackageInfo>::iterator end();
};


#endif

