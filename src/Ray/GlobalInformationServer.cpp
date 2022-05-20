#include"GlobalInformationServer.h"
#include"NodeManager.h"
#include"LinkManager.h"
#include"TCPController.h"
#include"ManagerLinkUnOrderMap.h"
#include"EventLoop.h"
#include "Base.h"

int resourceNumber=3;


const Time broadcastCycle=160000000;
extern int taskNumber;
extern bool iscreateTask;

Ray::GlobalInformationServer::GlobalInformationServer(){
}

Ray::GlobalInformationServer::GlobalInformationServer(NodeID id):broadcastCycle_(broadcastCycle),localNodeID_(id),
    broadStatus_(true),acceptStatus(true){
}

void Ray::GlobalInformationServer::AcceptCall(std::shared_ptr<Node> nodeGCS,Connection connection){
    push(connection);
    auto readCallback=[nodeGCS,connection,this](Message message){
        //将message进行转换
        LocalResourceTable temp=*(LocalResourceTable*)message.buff;
        delete (LocalResourceTable*)message.buff;
        this->receiveLocalResourceInfo(connection.m_destID,temp);
    };
    nodeGCS->read(connection,std::bind(readCallback,std::placeholders::_1));
    nodeGCS->accept([this,nodeGCS](Connection connect){
        this->AcceptCall(nodeGCS,connect);
    },Serverport::GlobalInfomationPort);
    //nodeGCS->accept(std::bind(&GlobalInformationServer::AcceptCall,this,nodeGCS,std::placeholders::_1)); 
}


void Ray::GlobalInformationServer::start(){
    std::shared_ptr<Node> nodeGCS=NodeManager::getIntance().searchNode(localNodeID_);
    nodeGCS->accept([this,nodeGCS](Connection connection){
        this->AcceptCall(nodeGCS,connection);
    },Serverport::GlobalInfomationPort);
    //broadcastGlobalResourceInfo();
}

void Ray::GlobalInformationServer::startbroadcastGlobalR(){
    broadcastGlobalResourceInfo();
}

void Ray::GlobalInformationServer::receiveLocalResourceInfo(NodeID nodeID, const LocalResourceTable& table){
    globalTable_[nodeID]=table;
    //testGlobalData();
}

void Ray::GlobalInformationServer::broadcastGlobalResourceInfo(){
    std::shared_ptr<Node> nodeGCS=NodeManager::getIntance().searchNode(localNodeID_);
    int nodeNumber=connections_.size();
    std::shared_ptr<GlobalResourceTable> table=std::make_shared<GlobalResourceTable>(globalTable_);
    
    for(int i=0;i<nodeNumber;i++){
        Message message;
        message.bufflength=nodeNumber*(sizeof(NodeID)+resourceNumber*sizeof(float));
        message.size=10*10;
        message.buff=(void*)(new std::shared_ptr<GlobalResourceTable>(table));
        
        nodeGCS->write(connections_[i],message,nullptr);
    }

    // if(globalTable_.size()!=0){
    //     setBroadStatus_(false);
    // }

    EventLoop::getInstance().callAfter(broadcastCycle_,[this](){
        if(this->getStatus()){
            this->broadcastGlobalResourceInfo();
        }
    });
}