#include "SharedObjectStorage.h"
#include "DiskConfigAnalyse.h"
#include "CPUAndMemoryConfigAnalyse.h"
#include "DiskConfigAnalyse.h"
#include "InitConfigFileAnalyse.h"
#include "Computation.h"
#include "Disk.h"
#include "NetDirverCircle.h"
#include "NodeManager.h"
#include "NetDriverManager.h"
#include "ManagerLinkUnOrderMap.h"
#include "NetDriverPolicyFactory.h"
#include "ComputationTaskSchedulerFactory.h"
#include "DiskDriverFactory.h"
#include "ObjTableManager.h"
#include "NetDriver.h"
#include "LinkManager.h"

#include <gtest/gtest.h>
#include <random>

int nodeNumber = 5;
float memoryRatio;


std::vector<std::unordered_map<uint32_t, float>> resources={
    {{1,100},{2,101},{3,102}},
    {{1,8.0},{2,11.0},{3,18.6}},
    {{1,7.6},{2,11.5},{3,19.8}},
    {{1,20.8},{2,24.5},{3,30.8}},
    {{1,31.8},{2,31.5},{3,31.8}},
    {{1,31.8},{2,31.5},{3,31.8}},
};

TEST(RayTest, testshareStorage){
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

    //创建ObjectTable
    ObjTableManager* manager=new ObjTableManager();
    manager->sharedObjectptr_=std::make_shared<Ray::SharedObjectStorage>(1);
    manager->sharedObjectptr_->setFreeMemorySize(100);
    manager->sharedObjectptr_->setLocalSharedMemorySize(100);
    manager->sharedObjectptr_->setObjectTableManager(manager);
    
    int sum=0;
    int i;

    for(i=0;i<1000;i++){
        //生成一个ObjectHandle
        Ray::ObjectHandle handle=std::make_shared<Ray::Object>();
        handle->size_=9;
        if(100<sum+handle->size_){
            break;
        }
        sum+=handle->size_;

        Ray::ObjectTableManager::ObjectInfo info;
        if(i%3==0){
            manager->put(handle,2,true,i+1);
        }
        else if(i%3==1){
            manager->put(handle,0,true,i+1);
        }
        else{
            manager->put(handle,0,false,i+1);
        }
    }
    EXPECT_TRUE(100>=sum);

    size_t rest=100-sum;
    Ray::ObjectHandle obj=std::make_shared<Ray::Object>();
    obj->size_=rest+30;
    manager->put(obj,2,true,i+1);
}

int main(int argc, char* argv[]){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
