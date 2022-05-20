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

Ray::ResourceSet Dedup_CompareHash_Request({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 0.0 } });
Ray::ResourceSet Dedup_CleanDuplication_Request({ { CPU, 1.0 }, { MEM, 1.0 }, { PLASMA_MEM, 0.0 } });

void Dedup_Driver(NodeID nodeid,uint32_t objAmount){
    const uint32_t        objPerTask = 10;  // 单个任务的工作量

    for (uint32_t i = 0; i < objAmount; i += objPerTask) {
        Ray::RayEnvHandle  Dedup_CompareHash_Env  = std::make_shared<Ray::RayEnv>();
        auto          Dedup_CompareHash_Func = std::bind(Dedup_CompareHash,Dedup_CompareHash_Env, objPerTask);
        Ray::ObjectID id = rayServices[nodeid]->remote(Dedup_CompareHash_Request, Dedup_CompareHash_Func, objPerTask);
    }
    
}

void Dedup_CompareHash(Ray::RayEnvHandle env, uint32_t objAmount){
    NetInputTask netInTask;
    NodeID nodeid=env->service_->getNodeID();
    std::shared_ptr<Node> node = NodeManager::getIntance().searchNode(nodeid);
    netInTask.Size_ = objAmount * MetaPerObj;

    pLayers[nodeid]->submitNetInTask(netInTask, [env,node,objAmount,nodeid]() {
        unsigned int dup_number=objAmount*DupPercent;
        size_t       per_dup_size=(objAmount * MetaPerObj*DupPercent)/dup_number;
        for(int i=1;i<=dup_number;i++){
            //生成新的RayObject
            std::shared_ptr<Ray::Object> obj=std::make_shared<Ray::Object>();
            obj->size_=per_dup_size;
            NodeID nodeid=env->service_->getNodeID();
            Ray::ObjectID objID=tableManagers[nodeid]->put(obj);

            Ray::RayEnvHandle  Dedup_CleanDuplication_Env  = std::make_shared<Ray::RayEnv>();
            // auto Dedup_CleanDuplication_Func=[objID,Dedup_CleanDuplication_Env](){
            //     Dedup_CleanDuplication(Dedup_CleanDuplication_Env,objID);
            // };
            auto Dedup_CleanDuplication_Func=std::bind(Dedup_CleanDuplication,Dedup_CleanDuplication_Env,objID);
            Ray::ObjectID obj=env->service_->remote(Dedup_CleanDuplication_Request,Dedup_CleanDuplication_Func,objID);
        }
    });
}

void Dedup_CleanDuplication(Ray::RayEnvHandle env,Ray::ObjectID objID){
    uint32_t plogAmount    = objID.getObj()->size_ / MetaPerPlog;
    uint32_t plogTotalSize = UtilizationThreshold * plogAmount * PlogSize;  // Plog中有效数据大小
    // TODO：从P层读Plog的有效数据（网络传输）
    NetInputTask netInTask;
    netInTask.Size_ = plogTotalSize;
    NodeID nodeid=env->service_->getNodeID();

    pLayers[nodeid]->submitNetInTask(netInTask,[env,plogTotalSize,nodeid](){
        DiskIOTask::Direction direction_ = DiskIOTask::Direction::WRITE;
        std::shared_ptr<Node> node = NodeManager::getIntance().searchNode(nodeid);
        node->submitTask(direction_, plogTotalSize * 0.5, [env]() { env->reportFinish_(nullptr); });
        direction_ = DiskIOTask::Direction::READ;
        node->submitTask(direction_, plogTotalSize * 0.5, [env]() { env->reportFinish_(nullptr); });

        
    });
}