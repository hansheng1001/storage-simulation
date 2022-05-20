#ifndef MANAGERLINKUNORDERMAP_H
#define MANAGERLINKUNORDERMAP_H

#include"Base.h"

#include<unordered_map>
#include<memory>

class LinkManager;

class ManagerLinkUnOrderMap
{
    static ManagerLinkUnOrderMap _instance;
private:
    /* data */
    std::unordered_map<int,std::shared_ptr<LinkManager>> linkManager;
private:
    ManagerLinkUnOrderMap(/* args */);
public:
    ~ManagerLinkUnOrderMap();
    static ManagerLinkUnOrderMap& getIntance();
    size_t size();
    std::shared_ptr<LinkManager> searchLink(int id);
    void deleteLink(int id);
    void insertLink(NodeID nodeid,std::shared_ptr<LinkManager> link){
        linkManager.insert({nodeid,link});
    }
    std::shared_ptr<LinkManager>& operator[](const int& i);//这个专门用于插入

    auto begin(){
        return linkManager.begin();
    }

    auto end(){
        return linkManager.end();
    }
    void clear(){
        linkManager.clear();
    }
};

#endif