#include"Node.h"
#include"NodeManager.h"

#include<fcntl.h>
#include<iostream>
#include<unistd.h>


NodeID m_startID=0;

NodeManager NodeManager::instance_;

NodeManager::NodeManager(){
}

NodeManager::~NodeManager(){
    StopNodeManager();
}

void NodeManager::StopNodeManager(){
    //m_nodeInfo.clear();
    m_NodeManager.clear();
}

void NodeManager::deleteNode(int id){
    m_NodeManager.erase(id);
}

std::shared_ptr<Node> NodeManager::searchNode(int id){
    if(m_NodeManager.find(id)==m_NodeManager.end()){
        return nullptr;
    }
    return m_NodeManager.at(id);
}

void NodeManager::insert(std::shared_ptr<Node> node){
    m_NodeManager[m_startID]=node;
    node->setNodeID(m_startID);
    m_startID++;
}

NodeManager& NodeManager::getIntance(){
    return instance_;
}

std::shared_ptr<Node> &NodeManager::operator[](int i){
    return m_NodeManager[i];
}

size_t NodeManager::size(){
    return m_NodeManager.size();
}