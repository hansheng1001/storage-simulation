#include "GlobalInformationClient.h"
#include"RayBaseDefination.h"
#include"Node.h"
#include"NodeManager.h"
#include"Connection.h"
#include"EventLoop.h"
#include "Callbacks.h"

#include<memory>
#include<iostream>
#include<string>

extern int resourceNumber;
const Time heartbeatCycle=160000000;
                          //880001568
                          
                          

extern int taskNumber;
extern bool iscreateTask;

Ray::GlobalInformationClient::GlobalInformationClient(NodeID id,NodeID GCSNode):localNodeID_(id),
                                                                                GCSNodeID_(GCSNode),
                                                                                heartbeatCycle_(heartbeatCycle),
                                                                                sendStatus(true),
                                                                                isConnection_(false)
{
}
void Ray::GlobalInformationClient::setConnection(Connection connection){
    connection_=connection;
}

void Ray::GlobalInformationClient::startClient(CallBack call){
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    std::shared_ptr<Node> nodeGCS=NodeManager::getIntance().searchNode(GCSNodeID_);

    if(node==nullptr&&nodeGCS==nullptr){
        return;
    }
    node->listen();
    nodeGCS->listen();
    Port port=node->CreatePort();
    connection_=node->CreateConnection(localNodeID_,port,GCSNodeID_,0);
    auto connectCallback=[this,call](Connection connection){
        this->setConnectionStatus(true);
        this->setConnection(connection);
        if(call){
            call();
        }
    };
    node->connect(connection_,std::bind(connectCallback,std::placeholders::_1));
}

void Ray::GlobalInformationClient::init(){
    if(!isConnection_){
        startClient([this](){
            this->init();
        });
    }
    else{
        std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
        node->read(connection_,[this](Message message){
            std::shared_ptr<GlobalResourceTable> temp=*(std::shared_ptr<GlobalResourceTable>*)message.buff;
            delete (std::shared_ptr<GlobalResourceTable>*)message.buff;
            this->receiveGlobalResourceInfo(temp);
        });
    }
}

void Ray::GlobalInformationClient::startSendInfo(){
    sendLocalResourceInfo();
}

void Ray::GlobalInformationClient::sendLocalResourceInfo(){
    // if(taskNumber==0&&!isCreateTask){
    //     this->setStatus(false);
    // }
    //得到节点
    if(isConnection_){
        std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
        Message message;
        //LocalResourceTable localResourceTable=getLocalResourceCallback_();
        LocalResourceTable* localResourceTable=new LocalResourceTable(getLocalResourceCallback_());
        message.bufflength=sizeof(uint32_t)+resourceNumber*sizeof(float);
        message.size=100;
        message.buff=localResourceTable;
        node->write(connection_,message,nullptr);
    } 
    
    EventLoop::getInstance().callAfter(heartbeatCycle_,[this](){
        if(this->getStatus()){
            this->sendLocalResourceInfo();
        }
        //测试代码
        //this->setStatus(false);
    });
}

void Ray::GlobalInformationClient::setConnectionStatus(bool status){
    isConnection_=status;
}

void Ray::GlobalInformationClient::receiveGlobalResourceInfo(std::shared_ptr<GlobalResourceTable> table){
    
    updateGlobalResourceCallback_(table);
    //测试代码
    //setStatus(false);
}

void Ray::GlobalInformationClient::setGetLocalResourceCallback(std::function<LocalResourceTable ()> call){
    getLocalResourceCallback_=call;
}

void Ray::GlobalInformationClient::setUpdateGlobalResourceCallback(std::function<void (std::shared_ptr<GlobalResourceTable>)> call){
    updateGlobalResourceCallback_=call;
}