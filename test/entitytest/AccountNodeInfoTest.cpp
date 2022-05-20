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

#include <memory>
#include <vector>

#include"EventLoop.h"
#include"Event.h"

#include "NodeMonitor.h"
#include "AccountNodeInfo.h"

#include <memory>
#include <vector>
#include <fstream>
#include <random>

#include "../TaskDriver.h"

const int nodeNumber = 5;
bool iscreateTask=true;

TEST(AccountNodeInfoWithTask, testAccountNodeInfo)
{
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

    for (int i = 0; i < nodeNumber; i++)
    {
        std::shared_ptr<Node> node = std::make_shared<Node>();
        //节点的信息
        // CPU和memory信息
        ComputationArch arch;
        for (int j = 0; j < 3; j++)
        {
            struct CPU cpu = { test1["DataServer"].Sockets_[j].Cycle_,
                test1["DataServer"].Sockets_[j].CPI_,
                test1["DataServer"].Sockets_[j].CoreNum_,
                test1["DataServer"].Sockets_[j].Node_ };
            arch.sockets_.push_back(cpu);

            arch.memoryNodes_.push_back({ test1["DataServer"].MemoryNodes_[0].Size_,
                test1["DataServer"].MemoryNodes_[0].LatencyFactor_ });
        }
        arch.NUMALatencyFactor_ = test1["DataServer"].NUMALatencyFactor_;
        node->setComputationArch(arch);

        {
            Disk disk;
            disk.channelNum_ = test2["DataServer"].Disks_[0].ChannelNum_;
            disk.maxReadBandwidth_ = test2["DataServer"].Disks_[0].MaxReadBandwidth_;
            disk.maxReadBandWidthPerChannel_ = test2["DataServer"].Disks_[0].MaxReadBandWidthPerChannel_;
            disk.maxWriteBandWidth_ = test2["DataServer"].Disks_[0].MaxWriteBandWidth_;
            disk.maxWriteBandWidthPerChannel_ = test2["DataServer"].Disks_[0].MaxWriteBandWidthPerChannel_;
            disk.readBaseLatency_ = test2["DataServer"].Disks_[0].ReadBaseLatency_;
            disk.writeBaseLatency_ = test2["DataServer"].Disks_[0].WriteBaseLatency_;
            node->addDisk(disk);

            disk.channelNum_ = test2["DataServer"].Disks_[1].ChannelNum_;
            disk.maxReadBandwidth_ = test2["DataServer"].Disks_[1].MaxReadBandwidth_;
            disk.maxReadBandWidthPerChannel_ = test2["DataServer"].Disks_[1].MaxReadBandWidthPerChannel_;
            disk.maxWriteBandWidth_ = test2["DataServer"].Disks_[1].MaxWriteBandWidth_;
            disk.maxWriteBandWidthPerChannel_ = test2["DataServer"].Disks_[1].MaxWriteBandWidthPerChannel_;
            disk.readBaseLatency_ = test2["DataServer"].Disks_[1].ReadBaseLatency_;
            disk.writeBaseLatency_ = test2["DataServer"].Disks_[1].WriteBaseLatency_;
            node->addDisk(disk);

            disk.channelNum_ = test2["DataServer"].Disks_[2].ChannelNum_;
            disk.maxReadBandwidth_ = test2["DataServer"].Disks_[2].MaxReadBandwidth_;
            disk.maxReadBandWidthPerChannel_ = test2["DataServer"].Disks_[2].MaxReadBandWidthPerChannel_;
            disk.maxWriteBandWidth_ = test2["DataServer"].Disks_[2].MaxWriteBandWidth_;
            disk.maxWriteBandWidthPerChannel_ = test2["DataServer"].Disks_[2].MaxWriteBandWidthPerChannel_;
            disk.readBaseLatency_ = test2["DataServer"].Disks_[2].ReadBaseLatency_;
            disk.writeBaseLatency_ = test2["DataServer"].Disks_[2].WriteBaseLatency_;
            node->addDisk(disk);
        }
        // Disk的信息


        //网卡信息
        NetDirverCircle circle(
            test3["DataServer"].NetDriver_[0].SendDelay_,
            test3["DataServer"].NetDriver_[0].InternetDelay_,
            test3["DataServer"].NetDriver_[0].DownBandWidth_,
            test3["DataServer"].NetDriver_[0].UpBandWidth_);

        std::shared_ptr<NetDriver> netdriver = CreateNetDriver(circle, 256);

        NodeManager::getIntance().insert(node);
        NodeID id = node->getNodeID();
        NetDriverManager::getIntance().insert(id, netdriver);
        std::shared_ptr<LinkManager> link = std::make_shared<LinkManager>(id);
        ManagerLinkUnOrderMap::getIntance().insertLink(id, link);

        node->setNetDriver(netdriver);
        node->initScheduler([](const ComputationArch& arch) -> std::unique_ptr<ComputationTaskScheduler>
            {
                return CreateFIFOComputationTaskScheduler(arch);
            });
        //这里的memory的问题，并没有初始化
        node->initDiskDriver(
            [](const Disk& disk) -> std::unique_ptr<DiskDriver> { return CreateFIFODiskDriver(disk, true); });
    }

    std::ofstream of{ "NodeAccount.json", std::ios::trunc };
    NodeMonitor<ArchiveNodeInfo<std::ofstream>, std::ofstream> nm(std::move(of), 100);

    CPUUtilizationAnalyzer<ArchiveNodeInfo<std::ofstream>> cpuarchive;
    MemoryUtilizationAnalyzer<ArchiveNodeInfo<std::ofstream>> memoryarchive;
    DiskAnalyzer<ArchiveNodeInfo<std::ofstream>> diskarchive;
    NetUtilizationAnalyzer<ArchiveNodeInfo<std::ofstream>> netarchive;

    nm.addAnalyzer(cpuarchive);
    nm.addAnalyzer(memoryarchive);
    nm.addAnalyzer(diskarchive);
    nm.addAnalyzer(netarchive);

    nm.startMonitoring();

    EventLoop::getInstance().callAfter(25000, [&nm]()
        {
            nm.stopMonitoring();
        });

    ComputationTask ct;
    std::uniform_real_distribution realRandom(0.2, 0.7);
    std::default_random_engine e;

    //前台任务类型：1
    //后台任务类型：2
    //每周期后台任务数量：50
    //每周期前台任务数量：20
    //task高峰时，task任务数因子 3
    //task触发高峰的界限：0.3
    //MaxNodeID：4
    BasicTaskGenerator btg(1, 2, 5, 20, 10, 0.5, 4);
    auto submitFunc = [&ct,&realRandom,&e](NodeID node, unsigned TaskType)->void
    {
        ct.instructionNum_ = rand() % 10 + 10;
        ct.memoryInstructionRatio_ = realRandom(e);
        ct.memoryRequired_ = 134217728 + rand() % 2000000000;
        if (TaskType == 0)
        {
            NodeManager::getIntance().searchNode(node)->submitCPUTask(ct, []() { });
        }
        else if (TaskType == 1)
        {
            NodeManager::getIntance().searchNode(node)->submitCPUTask(ct, []() { });
        }
        else if (TaskType == 2)
        {
            NodeManager::getIntance().searchNode(node)->submitTask((DiskIOTask::Direction)(rand() & 1), 1024 * 1024 + rand() % 10000000, []() { }, rand() % 3);
        }
    };

    auto gentaskFunc = [&btg](NodeID& node, unsigned& type)->bool
    {
        return btg.genTask(node, type);
    };

    TaskDriver td(20000, 20, submitFunc, gentaskFunc);
    td.start();

    EventLoop::getInstance().ChooseEventToStart();


}


int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}