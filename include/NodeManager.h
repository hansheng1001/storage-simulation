#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include<map>
#include<memory>
#include<vector>
#include<string>
#include<unordered_map>
#include <algorithm>

#include"Callbacks.h"
#include"Base.h"

class Node;
class EventLoop;
class LinkManager;



class NodeManager:noncopyable
{
    static NodeManager instance_;
private:
    /* data */
    //std::vector<struct NodeInfo> m_nodeInfo;
    std::unordered_map<int,std::shared_ptr<Node>> m_NodeManager;
    std::string m_configureName;
private:
    NodeManager();
public:
    ~NodeManager();
    void StopNodeManager();

    void deleteNode(int );
    std::shared_ptr<Node> searchNode(int);
    void insert(std::shared_ptr<Node>);
    static NodeManager& getIntance();
    size_t size();
    std::shared_ptr<Node>& operator[](int);

    template<typename Func>
    void forEachNode(Func f){
        std::for_each(m_NodeManager.begin(), m_NodeManager.end(), [&f](std::unordered_map<const int,std::shared_ptr<Node>>::value_type& pNode){
            f(*(pNode.second));
        });
    }
    void clear(){
        m_NodeManager.clear();
    }
};


#endif


