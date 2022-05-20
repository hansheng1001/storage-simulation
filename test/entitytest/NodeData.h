#include"NodeManager.h"
#include"NetDirverCircle.h"
#include"Node.h"
#include"LinkManager.h"
#include"ManagerLinkUnOrderMap.h"
#include"NetDriverPolicyFactory.h"
#include"NetDriverManager.h"

#include<tuple>

int node[8][4]={
    {1,2,13,80},
    {3,4,13,75},
    {2,1,14,78},
    {3,1,16,90},
    {2,4,17,100},
    {1,3,18,60},
    {1,2,20,65},
    {3,1,12,72}
};

void createNode(){
    for(int i=0;i<8;i++){
        NetDirverCircle circle(node[i][0],node[i][1],node[i][2],node[i][3]);
        std::shared_ptr<Node> node=std::make_shared<Node>();
        
        std::shared_ptr<NetDriver> netdriver=CreateNetDriver(circle,256);


        NodeManager::getIntance().insert(node);
        NodeID id=node->getNodeID();
        NetDriverManager::getIntance().insert(id,netdriver);
        node->setNetDriver(netdriver);

        std::shared_ptr<LinkManager> link=std::make_shared<LinkManager>(id);
        ManagerLinkUnOrderMap::getIntance()[id]=link;
    }
}