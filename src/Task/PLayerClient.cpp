#include "PLayerClient.h"
#include "Node.h"
#include "NodeManager.h"
#include "BaseTask.h"


void PLayerClient::startClient(CallBack call){
    std::shared_ptr<Node> nodepLayer=NodeManager::getIntance().searchNode(pLayerID_);
    std::shared_ptr<Node> localNode=NodeManager::getIntance().searchNode(localNodeID_);

    if(nodepLayer&&localNode){
        return;
    }
	
    nodepLayer->listen();
    localNode->listen();
    Port port=localNode->CreatePort();
    connection_=localNode->CreateConnection(localNodeID_,port,pLayerID_,0);
    auto connectCallback=[this,call](Connection connection){
        this->setConnection(connection);
        this->setConnectionStataus(true);
        if(call){
            call();
        }
    };
    localNode->connect(connection_,std::bind(connectCallback,std::placeholders::_1));
}

void PLayerClient::submitNetInTask(NetInputTask input,CallBack call){
    if(!connectionStatus_){
        startClient([this,input,call](){
            this->submitNetInTask(input,call);
        });
    }
	
    std::shared_ptr<PMessage> msg=std::make_shared<PMessage>();
    msg->category=Read;
    msg->input=input;

    Message message;
    message.buff=msg.get();
    message.bufflength=8;
    message.size=1;
    std::shared_ptr<Node> localNode=NodeManager::getIntance().searchNode(localNodeID_);
    localNode->write(connection_,message,nullptr);
    localNode->read(connection_,[call](Message message){
        if(call!=nullptr){
            call();
        }
    });
}

void PLayerClient::submitNetOutTask(NetInputTask input,CallBack call){
    if(!connectionStatus_){
        startClient([this,input,call](){
            this->submitNetInTask(input,call);
        });
    }
    std::shared_ptr<PMessage> msg=std::make_shared<PMessage>();
    msg->category=Write;
    msg->input=input;

    Message message;
    message.buff=msg.get();
    message.bufflength=8;
    message.size=1;
    std::shared_ptr<Node> localNode=NodeManager::getIntance().searchNode(localNodeID_);
    localNode->write(connection_,message,nullptr);
}