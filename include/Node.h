#ifndef NODE_H
#define NODE_H

#include <memory>
#include <map>
#include <list>
#include <chrono>
#include <vector>
#include "Callbacks.h"
#include "Base.h"
#include "DiskDriver.h"
#include "Computation.h"
#include"NetDirverCircle.h"


class Event;
class InternetTimeCount;
class NetDriver;

//每个node都有一个function的函数列表，执行对应的逻辑
class Node{
private:
    NodeID      m_id;
    //计算
    ComputationArch m_arch;
    std::unique_ptr<ComputationTaskScheduler> m_scheduler;
    //磁盘I/O
    std::vector<Disk> m_disks;
    std::vector<std::shared_ptr<DiskDriver>> m_diskDrivers;

    //网卡
    std::shared_ptr<NetDriver>  m_netDriver;
    

    void addEventToSchedule(std::shared_ptr<Event> event);
public:
    Node();
    Node(const NodeID id);
    ~Node();

    //初始化操作
    void setNetInfo(const NetDirverCircle& netInfo);
    void setNodeID(NodeID);
    void setNetDriver(std::shared_ptr<NetDriver>);

    void setComputationArch(const ComputationArch& arch);
    void addDisk(const Disk& disk);
    
    using ComputationTaskSchedulerInitFunc = std::function<std::unique_ptr<ComputationTaskScheduler> (const ComputationArch&)>;
    using DiskDriverInitFunc = std::function<std::unique_ptr<DiskDriver> (const Disk&)>;

    void initScheduler(ComputationTaskSchedulerInitFunc createScheduler);
    void initDiskDriver(DiskDriverInitFunc createDiskDriver);

    void submitTask(DiskIOTask::Direction direction_,unsigned long IOLength_,CallBack call,int number=0);

    
    //网络API
    void listen();
    void listenObjectTable();
    void listenRayService();
    bool connect(Connection,ConnectBack);
    Connection accept(ConnectBack,int port=0);//将得到的fd进行映射，fd对应的是网络IO=true,磁盘I/O是false
    // Connection acceptObjectTable(ConnectBack,int port=1);//主要是修改这个
    // Connection acceptRayService(ConnectBack,int port=2);//主要是修改这个
    bool read(Connection,ReadBack);
    bool write(Connection,Message,CallBack);
    void close(Connection);

    Port CreatePort();
    Connection CreateConnection(NodeID,Port,NodeID,Port);
    void submitCPUTask(const ComputationTask& task,CallBack call){
        m_scheduler->submitCompuationTask(task,call);
    }

    //节点运行状态信息获取
    const ComputationArchRuntimeState& getComputationArchRuntimeState() const;
    const ComputationArch& getComputationArch() const{
        return m_arch;
    }
    
    
    unsigned getDiskNum() const {
        return m_disks.size();
    }
    const Disk& getDiskInfo(unsigned diskIndex) const {
        return m_disks[diskIndex];
    }
    const DiskRunTimeState& getDiskRunTimeState(unsigned diskIndex) const;
    const NodeID getNodeID() const {
        return m_id;
    }

    NetDirverCircle getNetDriverInfo() const;
    

    bool allocMemory(size_t size);
    float getMemorySize(){
        float memorySize=0;
        for(size_t i=0;i<m_arch.memoryNodes_.size();i++){
            memorySize+=m_arch.memoryNodes_[i].size_;
        }
        return memorySize;
    }   
};

#endif