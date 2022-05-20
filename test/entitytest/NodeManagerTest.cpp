#include"NodeManager.h"
#include"NodeData.h"
#include"NetDriver.h"
#include<gtest/gtest.h>

extern NodeID m_startID;


//这里要改
TEST(NodeManagerTest,ReadConfigure){
    createNode();
    //NodeManager::getIntance().loadNodeConfiguration();
    EXPECT_EQ(m_startID,NodeManager::getIntance().size());
}

TEST(NodeManagerTest,SearchNode){
    for(NodeID i=1;i<m_startID;i++){
        EXPECT_NE(NodeManager::getIntance()[i],nullptr);
    }
    
}

int main(int argc,char** argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS(); 
}