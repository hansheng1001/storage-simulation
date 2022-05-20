#include "PLayerServer.h"
#include "Node.h"
#include <memory>
#include "NodeManager.h"
#include "BaseTask.h"

void PLayerServer::init(){
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    node->accept([this](Connection connection){
        this->push_back(connection);
        if(this->status_){
            this->acceptMessage(connection);
        }
    });
}

void PLayerServer::acceptMessage(Connection connection){
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    node->read(connection,[this](Message message){
        this->handleMessage(message);
    });
}

void PLayerServer::handleMessage(Message message){
    
}