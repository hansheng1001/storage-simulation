#include"TCPController.h"
#include"Node.h"
#include"NodeManager.h"
#include"EventLoop.h"
#include"ManagerLinkUnOrderMap.h"
#include"LinkManager.h"
#include"NetDriver.h"

#include<gtest/gtest.h>
#include<string>
#include"NodeData.h"

const std::string writecahr="write";
const std::string readchar="read";


void connectCallBack(Connection conn){
    EXPECT_TRUE(conn==Connection(1,5,3,5));
}

void acceptCallBack(Connection conn){
    EXPECT_TRUE(conn==Connection(3,5,1,5));
}

void AfterconnectCallBack(Connection conn){
    EXPECT_TRUE(conn==Connection(1,6,3,6));
}

void beforeacceptCallBack(Connection conn){
    //EXPECT_TRUE(conn==Connection(3,2,1,2));
    EXPECT_TRUE(conn.m_sourceID=3);
    EXPECT_TRUE(conn.m_sourcePort=6);
    EXPECT_TRUE(conn.m_destID=1);
    EXPECT_TRUE(conn.m_destPort=6);
}

void readCallBack(Message message){
    EXPECT_STREQ((char*)message.buff,readchar.c_str());
}

TEST(TCPLinkTest,ConnectAndAccept){
    //NodeManager::getIntance().loadNodeConfiguration();
    createNode();
    std::shared_ptr<Node> nodeOne=NodeManager::getIntance().searchNode(1);
    ASSERT_NE(nodeOne,nullptr);
    nodeOne->listen();
    std::shared_ptr<Node> nodeThree=NodeManager::getIntance().searchNode(3);
    ASSERT_NE(nodeThree,nullptr);
    nodeThree->listen();
    Port port=nodeOne->CreatePort();


    Connection conn=nodeOne->CreateConnection(1,port,3,0);
    EXPECT_TRUE(conn==Connection(1,port,3,0));
    
    auto f=[nodeOne](Connection conn){
        connectCallBack(conn);
    };
    nodeOne->connect(conn,std::bind(f,std::placeholders::_1));

    auto f1=[nodeThree](Connection conn){
        acceptCallBack(conn);
        nodeThree->close(conn);
    };
    nodeThree->accept(std::bind(f1,std::placeholders::_1));
    //这里为什么没有被关闭了
    EventLoop::getInstance().ChooseEventToStart();
    EXPECT_EQ(ManagerLinkUnOrderMap::getIntance().searchLink(3)->getTCPLayer(5),nullptr);
    EXPECT_EQ(ManagerLinkUnOrderMap::getIntance().searchLink(1)->getTCPLayer(5),nullptr);
}

TEST(TCPLinkTest,AcceptAndConnect){
    std::shared_ptr<Node> nodeOne=NodeManager::getIntance().searchNode(1);
    EXPECT_NE(nodeOne,nullptr);
    std::shared_ptr<Node> nodeThree=NodeManager::getIntance().searchNode(3);
    EXPECT_NE(nodeThree,nullptr);
    Port port=nodeOne->CreatePort();

    Connection conn=nodeOne->CreateConnection(1,port,3,0);
    EXPECT_TRUE(conn==Connection(1,port,3,0));
    
    nodeThree->accept(std::bind(beforeacceptCallBack,std::placeholders::_1));
    nodeOne->connect(conn,std::bind(AfterconnectCallBack,std::placeholders::_1));
    EventLoop::getInstance().ChooseEventToStart();
}

TEST(TCPLinkTest,ReadAndWrite){
    std::shared_ptr<Node> nodeOne=NodeManager::getIntance().searchNode(1);
    std::shared_ptr<Node> nodeThree=NodeManager::getIntance().searchNode(3);
    Port port=nodeOne->CreatePort();
    
    Connection conn=nodeOne->CreateConnection(1,port,3,0);

    auto Write=[nodeOne](Connection conn){
       Message message;
       message.buff=(void*)writecahr.c_str();
       message.bufflength=6;
       message.size=256;
       auto writeAfteRead=[nodeOne](Connection conn){
           nodeOne->read(conn,std::bind(readCallBack,std::placeholders::_1));
       };
       nodeOne->write(conn,message,std::bind(writeAfteRead,conn));
    };
    nodeOne->connect(conn,std::bind(Write,std::placeholders::_1));
    auto Read=[nodeThree](Connection conn){
        auto f=[nodeThree,conn](Message message){
            EXPECT_STREQ((char*)message.buff,writecahr.c_str());
            message.buff=(void*)readchar.c_str();
            message.bufflength=5;
            message.size=256;
            nodeThree->write(conn,message,nullptr);
        };
        nodeThree->read(conn,std::bind(f,std::placeholders::_1));
    };
    nodeThree->accept(std::bind(Read,std::placeholders::_1));
    EventLoop::getInstance().ChooseEventToStart();
}

TEST(TCPLinkTest,ReadAfterWrite){
    std::shared_ptr<Node> nodeOne=NodeManager::getIntance().searchNode(1);
    std::shared_ptr<Node> nodeThree=NodeManager::getIntance().searchNode(3);
    Port port=nodeOne->CreatePort();
    
    Connection conn=nodeOne->CreateConnection(1,port,3,0);

    auto Read=[nodeThree](Connection conn){
        auto f=[nodeThree,conn](Message message){
            EXPECT_STREQ((char*)message.buff,writecahr.c_str());
            message.buff=(void*)readchar.c_str();
            message.bufflength=5;
            message.size=256;
            nodeThree->write(conn,message,nullptr);
        };
        nodeThree->read(conn,std::bind(f,std::placeholders::_1));
    };
    nodeThree->accept(std::bind(Read,std::placeholders::_1));

    auto Write=[nodeOne](Connection conn){
       Message message;
       message.buff=(void*)writecahr.c_str();
       message.bufflength=6;
       message.size=256;
       auto writeAfteRead=[nodeOne](Connection conn){
           nodeOne->read(conn,std::bind(readCallBack,std::placeholders::_1));
       };
       nodeOne->write(conn,message,std::bind(writeAfteRead,conn));
    };
    nodeOne->connect(conn,std::bind(Write,std::placeholders::_1));
    
    EventLoop::getInstance().ChooseEventToStart();
}

int main(int argc,char** argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS(); 
}