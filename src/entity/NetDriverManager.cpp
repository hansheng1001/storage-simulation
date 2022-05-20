#include"NetDriverManager.h"

NetDriverManager NetDriverManager::_instance;

NetDriverManager::NetDriverManager(){
}

NetDriverManager::~NetDriverManager(){
}

NetDriverManager& NetDriverManager::getIntance(){
    return _instance;
}

size_t NetDriverManager::size(){
    return m_netDriverManager.size();
}

std::shared_ptr<NetDriver> NetDriverManager::searchNetDriver(int id){
    return m_netDriverManager[id];
}

void NetDriverManager::insert(int id,std::shared_ptr<NetDriver> netDriver){
    m_netDriverManager[id]=netDriver;
}

void NetDriverManager::deleteNetDriver(int id){
    m_netDriverManager.erase(id);
}

void NetDriverManager::clear(){
    m_netDriverManager.clear();
}