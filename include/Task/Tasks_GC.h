#pragma once
#include "BaseTask.h"
#include "Computation.h"
#include "EventLoop.h"
#include "Node.h"
#include "NodeManager.h"
#include "ObjectTableManager.h"
#include "PUserObjectTable.h"
#include "RayAPI.h"
#include "RayBaseDefination.h"
#include "PLayerClient.h"

#include <vector>

extern std::vector<std::shared_ptr<Ray::RayService>> rayServices;
extern std::vector<std::shared_ptr<Ray::ObjectTableManager>> tableManagers;
extern std::vector<std::shared_ptr<PLayerClient>> pLayers;

//const NodeID IndexID = 0, PersistenceID = 1;  // 抽象I层和P层为某个带宽很大的节点

//模仿正常的磁盘读写和计算任务

//const Ray::ResourceSet GC_Driver_Request({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 0.0 } });  // 在解析日志时使用
Ray::ResourceSet GC_MarkDeletion_Request({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 0.0 } });
Ray::ResourceSet GC_MarkDeletion_Request({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 0.0 } });
Ray::ResourceSet GC_FilterPlog_Request({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 1.0 } });
Ray::ResourceSet GC_MoveData_Request({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 1.0 } });
Ray::ResourceSet GC_DeletePlog_Request({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 1.0 } });

// TODO：应该有一种方法获取函数本地节点的Node对象，否则无法提交计算任务

/// 负责分发任务
/// \param env: 本地节点Ray环境
/// \param objAmount: 本次作业要处理的HWObject数量
/// \param plogAmount: 本次作业要处理的Plog数量
void GC_Driver(NodeID nodeid, uint32_t objAmount, uint32_t plogAmount) {
    // 创建标记删除任务
    const uint32_t        objPerTask = 10;  // 单个任务的工作量
    std::shared_ptr<u_int64_t> relyNumber=std::make_shared<uint64_t>(0);
    std::vector<Ray::ObjectID> objIDs;
    for (uint32_t i = 0; i < objAmount; i += objPerTask) {
        Ray::RayEnvHandle  GC_MarkDeletion_Env  = std::make_shared<Ray::RayEnv>();
        auto          GC_MarkDeletion_Func = std::bind(GC_MarkDeletion, GC_MarkDeletion_Env, objPerTask);
        Ray::ObjectID id = rayServices[nodeid]->remote(GC_MarkDeletion_Request, GC_MarkDeletion_Func, objPerTask);
        (*relyNumber)++;
        objIDs.push_back(id);
    }
    auto GC_FilterPlogCall=[relyNumber,nodeid,plogAmount](){
        (*relyNumber)--;
        if((*relyNumber)==0){
            // 创建筛选低利用率Plog任务
            Ray::RayEnvHandle GC_FilterPlog_Env  = std::make_shared<Ray::RayEnv>();
            auto         GC_FilterPlog_Func = std::bind(GC_FilterPlog, GC_FilterPlog_Env, plogAmount);
            rayServices[nodeid]->remote(GC_FilterPlog_Request, GC_FilterPlog_Func, plogAmount);
        }
    };
    for(int i=0;i<objIDs.size();i++){
        int index=tableManagers[nodeid]->getInfo(objIDs[i].getKey()).index();
        Ray::ObjectTableManager::ObjectInfo info;
        if(index == 0){
            std::get<0>(tableManagers[nodeid]->getInfo(objIDs[i].getKey())).DependencyCallbackQueue_.push_back(GC_FilterPlogCall);
        }   
    }
}

/// 从I层获取HWObject元数据，筛选出需要删除的HWObject，然后更新I层的信息
/// \param env: 本地节点Ray环境
/// \param objAmount: 本任务要处理的HWObject数量
void GC_MarkDeletion(Ray::RayEnvHandle env, uint32_t objAmount) {
    // TODO：从I层读（网络传输）
    // 建议对I层读写和P层读写进行包装
    NetInputTask netInTask;
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node = NodeManager::getIntance().searchNode(nodeid);
    netInTask.Size_ = objAmount * MetaPerObj;
    pLayers[nodeid]->submitNetInTask(netInTask, [env,node,objAmount,nodeid]() {
        ComputationTask cpuTask;
        cpuTask.instructionNum_ = objAmount * 1000;
        cpuTask.memoryRequired_ = GC_MarkDeletion_Request.get(MEM);
        node->submitCPUTask(cpuTask, [env]() { env->reportFinish_(nullptr); });
        // TODO：写回I层（网络传输）
        NetInputTask netOutTask;
        netOutTask.Size_ = objAmount * MetaPerObj;
        pLayers[nodeid]->submitNetOutTask(netOutTask, nullptr);
    });
    // 对元数据打标记
    
}

/// 从I层获取Plog元数据，筛选出低利用率的Plog，然后创建RayObject
/// \param env: 本地节点Ray环境
/// \param plogAmount: 本任务要处理的Plog数量
void GC_FilterPlog(Ray::RayEnvHandle env, uint32_t plogAmount) {
    // TODO: node未知
    // std::shared_ptr<Node> node = NodeManager::getIntance().searchNode(???);
    // TODO：从I层读（网络传输）
    NetInputTask netInTask;
    netInTask.Size_ = plogAmount * MetaPerPlog;
    NodeID nodeid=env->service_->getNodeID();
    pLayers[nodeid]->submitNetInTask(netInTask, [env,plogAmount,nodeid]() {
        size_t totalPlogMeta = plogAmount * MetaPerPlog * GarbagePercent;  // 应该清理的Plog对应元数据总大小
        const size_t plogMetaPerTask = 1000;
        NodeID id_=env->service_->getNodeID();
        for (size_t i = 0; i < totalPlogMeta; i += plogMetaPerTask) {
            // 每筛选出一些低利用率的Plog，就创建新任务
            ComputationTask cpuTask;
            cpuTask.instructionNum_ = plogMetaPerTask * 20;
            cpuTask.memoryRequired_ = GC_FilterPlog_Request.get(MEM);
            Ray::Object rayPlogList;  // TODO:这里如何获取创建的对象的ID？
            rayPlogList.size_              = plogAmount * MetaPerPlog * GarbagePercent;
            Ray::ObjectHandle rayPlogListHandle = std::make_shared<Ray::Object>(rayPlogList);
            
            Ray::ObjectID objID=tableManagers[id_]->put(rayPlogListHandle);
            std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(nodeid);
            node->submitCPUTask(cpuTask, [env,rayPlogListHandle]() { env->reportFinish_(rayPlogListHandle); });

            // TODO：写回I层（网络传输）
            NetInputTask netOutTask;
            netOutTask.Size_ = plogAmount * MetaPerPlog;
            pLayers[nodeid]->submitNetOutTask(netOutTask, [env]() {});

            // 提交MoveData任务
            Ray::RayEnvHandle GC_MoveData_Env = std::make_shared<Ray::RayEnv>();
            auto         GC_MoveData_Func =
                std::bind(GC_MoveData, GC_MoveData_Env, objID);  // TODO:这里如何获取创建的对象的ID？
            env->service_->remote(GC_MoveData_Request, GC_MoveData_Func, objID);
        }
    });

    // 筛选低利用率的Plog，创建RayObject，提交GC_MoveData任务
}

/// 将低利用率的Plog的数据进行搬迁
/// \param env: 本地节点Ray环境
/// \param rayPlogListID: 本任务要搬迁的Plog队列
void GC_MoveData(Ray::RayEnvHandle env, Ray::ObjectID rayPlogListID) {
    // 算出需要搬迁的数据大小
    uint32_t plogAmount    = rayPlogListID.getObj()->size_ / MetaPerPlog;
    uint32_t plogTotalSize = UtilizationThreshold * plogAmount * PlogSize;  // Plog中有效数据大小
    // TODO：从P层读Plog的有效数据（网络传输）
    NetInputTask netInTask;
    netInTask.Size_ = plogTotalSize;
    NodeID nodeid=env->service_->getNodeID();
    pLayers[nodeid]->submitNetInTask(netInTask, [env,plogTotalSize,nodeid,rayPlogListID]() {
        // 因为有大量数据读上来，需要模拟磁盘读写
        DiskIOTask::Direction direction_ = DiskIOTask::Direction::WRITE;
        std::shared_ptr<Node> node = NodeManager::getIntance().searchNode(nodeid);
        node->submitTask(direction_, plogTotalSize * 0.5, [env]() { env->reportFinish_(nullptr); });
        direction_ = DiskIOTask::Direction::READ;
        node->submitTask(direction_, plogTotalSize * 0.5, [env]() { env->reportFinish_(nullptr); });

        // TODO：将有效数据写回P层（网络传输）
        NetInputTask netOutTask;
        netOutTask.Size_ = plogTotalSize;
        pLayers[nodeid]->submitNetOutTask(netOutTask, [env,rayPlogListID]() {
            Ray::RayEnvHandle GC_DeletePlog_Env = std::make_shared<Ray::RayEnv>();
            auto GC_DeletePlog_Func =
            std::bind(GC_DeletePlog, GC_DeletePlog_Env, rayPlogListID);  // TODO:这里如何获取创建的对象的ID？
            env->service_->remote(GC_DeletePlog_Request, GC_DeletePlog_Func, rayPlogListID);
        });

    // 提交DeletePlog任务
    });
}

/// 将空的Plog的数据进行回收
/// \param env: 本地节点Ray环境
/// \param rayPlogListID: 本任务要回收的Plog队列
void GC_DeletePlog(Ray::RayEnvHandle env, Ray::ObjectID rayPlogListID) {
    // TODO：将回收列表发给P层，由P层自行回收（网络传输）
    NetInputTask netOutTask1;
    netOutTask1.Size_ = rayPlogListID.getObj()->size_;
    NodeID nodeid=env->service_->getNodeID();
    pLayers[nodeid]->submitNetOutTask(netOutTask1, [env,nodeid,rayPlogListID]()mutable{
        // TODO：将回收列表发给I层，更新元数据（网络传输）
        NetInputTask netOutTask2;
        netOutTask2.Size_ = rayPlogListID.getObj()->size_;
        pLayers[nodeid]->submitNetOutTask(netOutTask2, [env]() {});
    });

    
}