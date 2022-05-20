#include "DiskConfigAnalyse.h"
#include "CPUAndMemoryConfigAnalyse.h"
#include "DiskConfigAnalyse.h"
#include "InitConfigFileAnalyse.h"

#include <gtest/gtest.h>

#include "Computation.h"
#include "Disk.h"
#include "NetDirverCircle.h"

#include "ComputationTaskSchedulerFactory.h"
#include "DiskDriverFactory.h"
#include "NetDriver.h"
#include "NetDriverManager.h"
#include "NetDriverPolicyFactory.h"
#include "Node.h"
#include "NodeManager.h"
#include "EventLoop.h"
#include "ManagerLinkUnOrderMap.h"
#include "LinkManager.h"

#include "GlobalInformationClient.h"
#include "GlobalInformationServer.h"
#include "RayBaseDefination.h"
#include "RayScheduler.h"
#include "ObjectTableManager.h"
#include "RayAPI.h"
#include "TaskExecutor.h"

#include "Tasks_test_ObjectInMemoryObjectLarger.h"

#include <memory>
#include <vector>

int nodeNumber=5;
float memoryRatio=0.4;

std::vector<std::unordered_map<uint32_t, float>> resources={
    {{1,100},{2,101},{3,102}},
    {{1,8.0},{2,11.0},{3,128}},
    {{1,7.6},{2,11.5},{3,19.8}},
    {{1,20.8},{2,24.5},{3,30.8}},
    {{1,31.8},{2,31.5},{3,31.8}},
    {{1,31.8},{2,31.5},{3,31.8}},
};

void testReceiveGlobalData(std::vector<std::unordered_map<uint32_t, float>> data,std::unordered_map<NodeID, Ray::LocalResourceTable> map){
    ASSERT_NE(map.size(),0);
    auto begin=map.begin();
    auto end=map.end();
    for(;begin!=end;begin++){
        NodeID id=begin->first;
        EXPECT_EQ(data[id][1],map[id][1]);
        EXPECT_EQ(data[id][2],map[id][2]);
        EXPECT_EQ(data[id][3],map[id][3]);
    }
}

void testPerNodeResource(std::unordered_map<uint32_t, float> testData,std::unordered_map<uint32_t, float> nodeData){
    EXPECT_EQ(testData[1],nodeData[1]);
    EXPECT_EQ(testData[2],nodeData[2]);
    EXPECT_EQ(testData[3],nodeData[3]);
}

TEST(SetNodeInfo, testGlobalInfo) {
    //设置每个节点的参数
    bool              OpenFileFlag = false;
    ConfigFileAnalyse CFA;

    auto& test1 = ServerConfigStoreBuffer::ServerCPUAndMemoryConfigMap_;
    auto& test2 = ServerConfigStoreBuffer::ServerDiskConfigMap_;
    auto& test3 = ServerConfigStoreBuffer::ServerNetDriverConfigMap_;

    OpenFileFlag = CFA.Init("Node.json");
    ASSERT_TRUE(OpenFileFlag);
    if (!OpenFileFlag)
        return;

    CFA.Analyse();

    for (int i = 0; i < nodeNumber; i++) {
        //节点的信息
        // CPU和memory信息
        ComputationArch arch;
        for (int j = 0; j < 3; j++) {
            struct CPU cpu={test1["DataServer"].Sockets_[j].Cycle_,
                            test1["DataServer"].Sockets_[j].CPI_,
                            test1["DataServer"].Sockets_[j].CoreNum_,
                            test1["DataServer"].Sockets_[j].Node_};
            arch.sockets_.push_back(cpu);
            
            arch.memoryNodes_.push_back({test1["DataServer"].MemoryNodes_[0].Size_,
                                        test1["DataServer"].MemoryNodes_[0].LatencyFactor_});
        }
        arch.NUMALatencyFactor_ = test1["DataServer"].NUMALatencyFactor_;

        //要将arch插入到node当中




        // Disk的信息
        Disk disk;
        disk.channelNum_                  = test2["DataServer"].Disks_[0].ChannelNum_;
        disk.maxReadBandwidth_            = test2["DataServer"].Disks_[0].MaxReadBandwidth_;
        disk.maxReadBandWidthPerChannel_  = test2["DataServer"].Disks_[0].MaxReadBandWidthPerChannel_;
        disk.maxWriteBandWidth_           = test2["DataServer"].Disks_[0].MaxWriteBandWidth_;
        disk.maxWriteBandWidthPerChannel_ = test2["DataServer"].Disks_[0].MaxWriteBandWidthPerChannel_;
        disk.readBaseLatency_             = test2["DataServer"].Disks_[0].ReadBaseLatency_;
        disk.writeBaseLatency_            = test2["DataServer"].Disks_[0].WriteBaseLatency_;
        //网卡信息
        NetDirverCircle circle(
            test3["DataServer"].NetDriver_[0].SendDelay_, 
            test3["DataServer"].NetDriver_[0].InternetDelay_,
            test3["DataServer"].NetDriver_[0].DownBandWidth_, 
            test3["DataServer"].NetDriver_[0].UpBandWidth_);

        std::shared_ptr<Node> node = std::make_shared<Node>();
        node->addDisk(disk);
        node->setComputationArch(arch);
        
        std::shared_ptr<NetDriver> netdriver = CreateNetDriver(circle, 256);

        NodeManager::getIntance().insert(node);
        NodeID id = node->getNodeID();
        NetDriverManager::getIntance().insert(id, netdriver);
        std::shared_ptr<LinkManager> link=std::make_shared<LinkManager>(id);
        ManagerLinkUnOrderMap::getIntance().insertLink(id,link);

        node->setNetDriver(netdriver);
        node->initScheduler([](const ComputationArch& arch) -> std::unique_ptr<ComputationTaskScheduler> {
            return CreateFIFOComputationTaskScheduler(arch);
        });
        //这里的memory的问题，并没有初始化
        node->initDiskDriver(
            [](const Disk& disk) -> std::unique_ptr<DiskDriver> { return CreateFIFODiskDriver(disk, true); });
    }
    ASSERT_EQ(ManagerLinkUnOrderMap::getIntance().size(),nodeNumber);
    //创建GlobalInformationClient,这里的Server是GCS节点是0节点
    
    //
    for (int i = 0; i < nodeNumber; i++) {
        //创建RayService
        std::shared_ptr<Ray::RayService> rayService=std::make_shared<Ray::RayService>(i);
        rayService->setPtr(i);
        rayService->setLocalResourceTable(resources[i]);
        rayServices.push_back(rayService);


        if(i==0){
            informationServer.setLocalNodeID(0);
            informationServer.testdata=[](std::unordered_map<NodeID, Ray::LocalResourceTable> map){
                testReceiveGlobalData(resources,map);
            };
            informationClients.push_back(nullptr);
        }
        else{
            std::shared_ptr<Ray::GlobalInformationClient> informationClient=std::make_shared<Ray::GlobalInformationClient>(i,0);
            //设置infomationClients的回调函数
            rayService->setInfomationClientCallBack(informationClient);
            informationClients.push_back(informationClient);
        }
    }
    for(int i=0;i<nodeNumber;i++){
        if(i==0){
            EXPECT_EQ(informationClients[i],nullptr);
            continue;
        }
        EXPECT_NE(informationClients[i],nullptr);
    }
    
    for(int i=0;i<nodeNumber;i++){
        if(i!=0){
            informationClients[i]->init();
        }
    }
    
    informationServer.start();

    
    for(int i=0;i<nodeNumber;i++){
        if(i!=0){
            informationClients[i]->startSendInfo();
        }
    }
    
    informationServer.startbroadcastGlobalR();


    //开始测试ray框架中的代码
    
    TestTaskSuccess_Driver(1,3,4,1000);

    EventLoop::getInstance().ChooseEventToStart();
    //开始释放函数
    //1.释放每一个ObjectTableManager所有的ObjectInfo
    //2.停止所有的localworkerq
    //3.释放所有的

    //这里有一个问题就是退出的时候，会将一些函数加入到EventLoop中，导致没有执行
    for (size_t i = 0; i <  rayServices.size(); i++){
        rayServices[i]->exit();
    }
    //EventLoop::getInstance().ChooseEventToStart();
    //先将LinkManager删除
    // ManagerLinkUnOrderMap::getIntance().clear();
    // NodeManager::getIntance().clear();
}

int main(int argc, char* argv[]){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}