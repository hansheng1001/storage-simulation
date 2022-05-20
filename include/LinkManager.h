#ifndef LINKMANAGE_H
#define LINKMANAGE_H

#include<unordered_map>
#include<memory>
#include"Base.h"
#include"Node.h"

class TCPController;
class NodeManager;


class LinkManager{
private:
    std::unordered_map<Port,std::shared_ptr<TCPController>> m_portToTcpLink;
	
    AddConnection m_addconnect;

    Port port;
    
    Port m_acceptPort;//记录accept对应的port
    bool acceptStatus;

    bool RayServiceStatus;
    bool ObjectTableStatus;

public:
    LinkManager(NodeID id);
    ~LinkManager();
    Port CreatePort();
    void addPortLink(Port,std::shared_ptr<TCPController>);
    std::shared_ptr<Node> getDataSource(NodeID);
    void setAddConnection(AddConnection call){
        m_addconnect=call;
    }

    void listen();
    void listenObjectTable();
    void listenRayService();
    void connect(Connection connection,ConnectBack call);
    Connection accept(ConnectBack,int);//将得到的fd进行映射，fd对应的是网络IO=true,磁盘I/O是false
    bool read(Connection connection,ReadBack);
    bool write(Connection,Message,CallBack);
    bool close(Connection);

    void ReplyConnect(Connection& connection);
    void setReplyConnect(ConnectAtferBack);
    bool deleteConnect(Connection);
    void clear(){
        m_portToTcpLink.clear();
    }

    std::shared_ptr<TCPController> getTCPLayer(Port);
};

#endif