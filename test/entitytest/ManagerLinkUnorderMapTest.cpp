#include"ManagerLinkUnOrderMap.h"
#include"Node.h"
#include"NodeManager.h"
#include"LinkManager.h"
#include"NodeData.h"
#include"NetDriver.h"

#include<gtest/gtest.h>

extern NodeID m_startID;

//这里要改

TEST(ManagerLinkUnOrderMap,AllFunctionTest){
    createNode();
    //NodeManager::getIntance().loadNodeConfiguration();
    EXPECT_EQ(m_startID,NodeManager::getIntance().size());
    EXPECT_EQ(m_startID,ManagerLinkUnOrderMap::getIntance().size());
    for(NodeID i=1;i<m_startID;i++){
        EXPECT_NE(ManagerLinkUnOrderMap::getIntance()[i],nullptr);
    }
}


int main(int argc,char** argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS(); 
}

