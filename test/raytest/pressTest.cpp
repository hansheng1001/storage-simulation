
#include "DiskConfigAnalyse.h"
#include "CPUAndMemoryConfigAnalyse.h"
#include "DiskConfigAnalyse.h"
#include "InitConfigFileAnalyse.h"

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

#include "RayTaskTest.h"
#include "AccountTaskInfo.h"
#include "AccountNodeInfo.h"

#include "../TaskDriver.h"

#include <memory>
#include <vector>
#include <gtest/gtest.h>

std::vector<std::unordered_map<uint32_t, float>> resources;
NodeMonitor<ArchiveNodeInfo<std::ofstream>, std::ofstream> *nmPtr;

//设置任务数量
int taskNumber=0;
bool iscreateTask=true;

int nodeNumber=10000;
float memoryRatio=0.2;

void stopGlobalInfomation(){
    taskNumber--;
    if(taskNumber==1){
        //std::cout << "/* message */" << std::endl;
    }
    if(taskNumber==0&&!iscreateTask){
        informationServer.setBroadStatus_(false);
        for(size_t i=0;i<informationClients.size();i++){
            if(i!=0){
                informationClients[i]->setStatus(false);
            }
        }
        nmPtr->stopMonitoring();
    }
}

TEST(PressTest,press){
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
        std::shared_ptr<Node> node = std::make_shared<Node>();
        //节点的信息
        // CPU和memory信息
        ComputationArch arch;
        unsigned long cpuNumber=0,memorySize=0;
		
        for (int j = 0; j < 2; j++) {
            struct CPU cpu={test1["DataServer"].Sockets_[j].Cycle_,
                            test1["DataServer"].Sockets_[j].CPI_,
                            test1["DataServer"].Sockets_[j].CoreNum_,
                            test1["DataServer"].Sockets_[j].Node_};
			
            arch.sockets_.push_back(cpu);
            
            arch.memoryNodes_.push_back({test1["DataServer"].MemoryNodes_[j].Size_,
                                        test1["DataServer"].MemoryNodes_[j].LatencyFactor_});
			
            cpuNumber += test1["DataServer"].Sockets_[j].CoreNum_;
            memorySize += test1["DataServer"].MemoryNodes_[j].Size_;
        }
		
        arch.NUMALatencyFactor_ = test1["DataServer"].NUMALatencyFactor_;
		
        resources.push_back({{1,cpuNumber},{2,memorySize*(1-memoryRatio)},{3,memorySize*memoryRatio}});
		
        //要将arch插入到node当中
        // Disk的信息
        for(int i=0;i<3;i++){
            Disk disk;
            disk.channelNum_                  = test2["DataServer"].Disks_[i].ChannelNum_;
            disk.maxReadBandwidth_            = test2["DataServer"].Disks_[i].MaxReadBandwidth_;
            disk.maxReadBandWidthPerChannel_  = test2["DataServer"].Disks_[i].MaxReadBandWidthPerChannel_;
            disk.maxWriteBandWidth_           = test2["DataServer"].Disks_[i].MaxWriteBandWidth_;
            disk.maxWriteBandWidthPerChannel_ = test2["DataServer"].Disks_[i].MaxWriteBandWidthPerChannel_;
            disk.readBaseLatency_             = test2["DataServer"].Disks_[i].ReadBaseLatency_;
            disk.writeBaseLatency_            = test2["DataServer"].Disks_[i].WriteBaseLatency_;
            node->addDisk(disk);
        }
        
        //网卡信息
        NetDirverCircle circle(
            test3["DataServer"].NetDriver_[0].SendDelay_, 
            test3["DataServer"].NetDriver_[0].InternetDelay_,
            test3["DataServer"].NetDriver_[0].DownBandWidth_, 
            test3["DataServer"].NetDriver_[0].UpBandWidth_);
        
        node->setComputationArch(arch);
        
        std::shared_ptr<NetDriver> netdriver = CreateNetDriver(circle, 1024);
        node->setNetDriver(netdriver);
        NodeManager::getIntance().insert(node);
        NodeID id = node->getNodeID();
        NetDriverManager::getIntance().insert(id, netdriver);
        std::shared_ptr<LinkManager> link=std::make_shared<LinkManager>(id);
        ManagerLinkUnOrderMap::getIntance().insertLink(id,link);
        
        node->initScheduler([](const ComputationArch& arch) -> std::unique_ptr<ComputationTaskScheduler> {
            return CreateFIFOComputationTaskScheduler(arch);
        });
        
        node->initDiskDriver(
            [](const Disk& disk) -> std::unique_ptr<DiskDriver> { return CreateFIFODiskDriver(disk, true); });
    }
	
    ASSERT_EQ(ManagerLinkUnOrderMap::getIntance().size(),nodeNumber);
    ASSERT_EQ(NodeManager::getIntance().size(),nodeNumber);

    for (int i = 0; i < nodeNumber; i++) {
        //创建RayService
        std::shared_ptr<Ray::RayService> rayService=std::make_shared<Ray::RayService>(i);
        rayService->setPtr(i);
        rayService->setLocalResourceTable(resources[i]);
		
        rayServices.push_back(rayService);
		
        if(i==0){
            informationServer.setLocalNodeID(0);
            informationClients.push_back(nullptr);
        }
        else{
            std::shared_ptr<Ray::GlobalInformationClient> informationClient=std::make_shared<Ray::GlobalInformationClient>(i,0);
            rayService->setInfomationClientCallBack(informationClient);
            informationClients.push_back(informationClient);
        }
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

        //启动Node信息统计
    std::ofstream AccountNodeInfoFile{ "NodeAccount.json", std::ios::trunc };
    NodeMonitor<ArchiveNodeInfo<std::ofstream>, std::ofstream> nm(std::move(AccountNodeInfoFile), 8000000);
                                                                                                   

                                                                                                                  
    CPUUtilizationAnalyzer<ArchiveNodeInfo<std::ofstream>> cpuarchive;
    MemoryUtilizationAnalyzer<ArchiveNodeInfo<std::ofstream>> memoryarchive;
    DiskAnalyzer<ArchiveNodeInfo<std::ofstream>> diskarchive;
    NetUtilizationAnalyzer<ArchiveNodeInfo<std::ofstream>> netarchive;

    nm.addAnalyzer(cpuarchive);
    nm.addAnalyzer(memoryarchive);
    nm.addAnalyzer(diskarchive);
    nm.addAnalyzer(netarchive);

    nm.startMonitoring();
    nmPtr=&nm;

    //启动task信息统计
    std::ofstream AccountTaskInfoFile{ "TaskAccount.json", std::ios::trunc };
    AccountTaskInfo::getInstance().SetOutputStream(&AccountTaskInfoFile);


    BasicTaskGenerator btg(3, 4, 7000, 7000, 2, 0.7, nodeNumber - 1);
	
    auto submitFunc=[](NodeID node, unsigned TaskType)->void{
        std::uniform_int_distribution IOsize(50000,100000);
        std::uniform_int_distribution randomNum(10,100);
        std::uniform_real_distribution<float> instructionRandom(0.2, 0.7);
        std::default_random_engine e;
		
        switch (TaskType)
        {
        case 0:
            //前台CPU任务
            ComputationTask ct;
            ct.instructionNum_ = 100000000;
            ct.memoryInstructionRatio_ = instructionRandom(e);
            ct.memoryRequired_ = 134217728 + rand() % 2000000000;
			
            NodeManager::getIntance().searchNode(node)->submitCPUTask(ct, []() {
                stopGlobalInfomation();
             });
            break;
        case 1:{
            //前台写任务
            DiskIOTask::Direction direction_=DiskIOTask::Direction::WRITE;
            NodeManager::getIntance().searchNode(node)->submitTask(direction_,IOsize(e),[](){
                stopGlobalInfomation();
            });
            break;
        }
        case 2:{
            DiskIOTask::Direction direction_=DiskIOTask::Direction::READ;
            NodeManager::getIntance().searchNode(node)->submitTask(direction_,IOsize(e),[](){
                stopGlobalInfomation();
            });
            break;
        }
        case 3:{
            int testA=randomNum(e);
            int testB=randomNum(e);
            rayServices[node]->remote(Task_Test_One,TaskOne,testA,testB);
            break;
        }
        case 4:
            rayServices[node]->remote(Task_Test_Two,TaskTwo);
            break;
        case 5:
            rayServices[node]->remote(Task_Test_Three,TaskThree,IOsize(e));
            break;
        case 6:{
            int testA=randomNum(e);
            int testB=randomNum(e);
            rayServices[node]->remote(Task_Test_Four,TaskFour,testA,testB);
            break;
        }
        }
        taskNumber++;
    };
	
    auto gentaskFunc = [&btg](NodeID& node, unsigned& type)->bool
    {
        return btg.genTask(node, type);
    };
	
    TaskDriver td(96000000ul*10, 96000000ul, submitFunc, gentaskFunc);
                            
    td.start();
	
    EventLoop::getInstance().ChooseEventToStart();
    for (size_t i = 0; i <  rayServices.size(); i++){
        rayServices[i]->exit();
    }
	
    AccountTaskInfoFile.close();
    //std::cout << EventLoop::getInstance().GetCurrentTime() << std::endl;
}

int main(int argc, char* argv[]){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

