#include"RayAPI.h"
#include"ObjectTableManager.h"
#include"RayBaseDefination.h"
#include"TaskExecutor.h"
#include "Base.h"
#include "InputObjectReader.h"

#include"NodeManager.h"
#include "Node.h"

#include "EventLoop.h"
#include "AccountTaskInfo.h"

#include<memory>

extern  float memoryRatio;

void Ray::RayService::reportResult(ObjectID objID, ObjectHandle obj){
    localObjTableManager_->reportResult(objID,obj);
}

void Ray::RayService::acceptTask(RayTaskHandle task)
{
    //3.任务被调度器调度统计
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskScheduled, task->taskID_, task->startNodeID_, EventLoop::getInstance().GetCurrentTime(), localNodeID_);
    //这里要检查Object是否已经在本地了
    localWorker_->submitTask(task);
    //env.reportFinish_=std::bind();
}

void Ray::RayService::deleteObj(ObjectKey& key){
    localObjTableManager_->deletObj(key);
}   

Ray::ObjectID Ray::RayService::put(ObjectHandle obj){
    return localObjTableManager_->put(obj);
}

void Ray::RayService::setInfomationClientCallBack(std::shared_ptr<GlobalInformationClient> client){
    localScheduler_->setGlobalInformationClientCallBack(client);
}

void Ray::RayService::setPtr(NodeID id){
    //创建各个函数的
    localScheduler_=std::make_unique<Ray::RayScheduler>(id);
    localObjTableManager_=std::make_unique<Ray::ObjectTableManager>(id);
    localWorker_=std::make_unique<Ray::TaskExecutor>(this);

    //设置RayScheduler的回调函数
    localScheduler_->execWhenAcceptNewTaskFromRemote([this](Ray::RayTaskHandle taskHandle){
        this->acceptTask(taskHandle);
    });
    localScheduler_->setObjectTableManager(localObjTableManager_.get());
    
    localScheduler_->setSchedulePolicy();

    //设置ObjectTableManager的回调函数
    //得到节点内存的大小
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    float memorySize=node->getMemorySize();
    localObjTableManager_->setRayService(this);
    localObjTableManager_->setSharedObjectStorage(memoryRatio,memorySize);

    //设置TaskExecutor的回调函数
    ObjectTableManager* localObjTableManager=localObjTableManager_.get();
    if(localObjTableManager==nullptr){
        std::cout << "can't get localObjTableManager" << std::endl;
    }
    RayScheduler* localScheduler=localScheduler_.get();
    if(localScheduler==nullptr){
        std::cout << "can't get localScheduler" << std::endl;
    }
    localWorker_->setAllocResourceCallback([localScheduler](const ResourceSet& set)->bool{
        if(localScheduler==nullptr){
            std::cout << "can't get localScheduler" << std::endl;
            return false;
        }
        localScheduler->allocResource(set);
        return true;
    });
    //localWorker_->setAllocResourceCallback(std::bind(RayScheduler::allocResource,localScheduler_,std::placeholders::_1));
    localWorker_->setFreeResourceCallback([localScheduler](const ResourceSet& set){
        if(localScheduler==nullptr){
            std::cout << "can't get localScheduler" << std::endl;
        }
        localScheduler->freeResource(set);
    });
    auto callBack=[localObjTableManager](const std::vector<ObjectID>& objs, std::function<void()> callback){
        if(localObjTableManager==nullptr){
            std::cout << "can't get localObjTableManager" << std::endl;
        }
        InputObjectReader::execWhenAllInputObjectReady(localObjTableManager,objs,callback);
    };
    localWorker_->setWaitInputObjectCallback(callBack);
    localWorker_->setReportResultCallback([this](ObjectID objID, ObjectHandle obj){
        this->reportResult(objID,obj);
    });
    localWorker_->setReduceObjectInTaskUsingCount([localObjTableManager](Ray::RayTaskHandle task){
        localObjTableManager->reduceObjectUsingCountInRayTask(task);
    });

    //RayScheduler* localScheduler=localScheduler_.get();
    if(localScheduler==nullptr){
        std::cout << "localScheduler can't get" << std::endl;
    }
    localWorker_->setReportTaskDataReady([localScheduler](NodeID nodeID, RayTaskHandle task){
        if(localScheduler==nullptr){
            std::cout << "localScheduler can't get" << std::endl;
        }
        localScheduler->sendTaskDataReadyToRemoteScheduler(nodeID,task);
    });
    localWorker_->setReportTaskExecuateFinish([localScheduler](NodeID nodeID, RayTaskHandle task){
        if(localScheduler==nullptr){
            std::cout << "localScheduler can't get" << std::endl;
        }
        localScheduler->sendTaskExecuateFinish(nodeID,task);
    });


    //启动服务
    localObjTableManager_->startServer();
    localScheduler_->startServer();
    
}

Ray::ObjectHandle Ray::RayService::getObj(ObjectID objID){
    return localObjTableManager_->get(objID);
}

void Ray::RayService::setLocalResourceTable(LocalResourceTable table){
    localScheduler_->setLocalResourceTable(table);
    localScheduler_->setTable_(table);
}

void Ray::RayService::exit(){
    //首先删除所有的Object
    localObjTableManager_->exit();
    localScheduler_->exit();
    localWorker_->exit();
}

NodeID Ray::RayService::getNodeID() const {
    return localNodeID_;
}

Ray::ObjectTableManager* Ray::RayService::getObjTableManager(){
    return localObjTableManager_.get();
}