#include"ManagerLinkUnOrderMap.h"

ManagerLinkUnOrderMap ManagerLinkUnOrderMap::_instance;

ManagerLinkUnOrderMap::ManagerLinkUnOrderMap(){
}

ManagerLinkUnOrderMap::~ManagerLinkUnOrderMap(){

}

std::shared_ptr<LinkManager>& ManagerLinkUnOrderMap::operator[](const int& i){
    return linkManager[i];
}

ManagerLinkUnOrderMap& ManagerLinkUnOrderMap::getIntance(){
    return _instance;
}

size_t ManagerLinkUnOrderMap::size(){
    return linkManager.size();
}

std::shared_ptr<LinkManager> ManagerLinkUnOrderMap::searchLink(int id){
    if(linkManager.find(id)==linkManager.end()){
        return nullptr;
    }
    return linkManager.at(id);
}

void ManagerLinkUnOrderMap::deleteLink(int id){
    linkManager.erase(id);
}