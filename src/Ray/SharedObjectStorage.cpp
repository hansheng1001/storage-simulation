#include"RayBaseDefination.h"
#include"ObjectTableManager.h"
#include"SharedObjectStorage.h"
#include"NodeManager.h"
#include"EventLoop.h"
#include"Node.h"


#include "DiskDriver.h"



// const size_t localSharedMemorySize_=20;

// void Ray::SharedObjectStorage::start(){
//     std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
//     node->allocMemory(localSharedMemorySize_);
//     freeMemorySize_=localSharedMemorySize_;
// }


bool Ray::SharedObjectStorage::addObject(size_t objSize){
    if(freeMemorySize_<objSize){
        reclaim(objSize);
        if(freeMemorySize_>objSize){
            freeMemorySize_-=objSize;
            return true;
        }
        else{
            //这里要加是不是清理的一定的Object，空间还是不够，所以这样写
            EventLoop::getInstance().addCallBack([this,objSize](){
                this->addObject(objSize);
            });
            return false;
        }
    }
    else{
        //已测
        freeMemorySize_-=objSize;
        return true;
    }
    return true;
}

void Ray::SharedObjectStorage::removeObject(size_t objSize){
    freeMemorySize_+=objSize;
}

void Ray::SharedObjectStorage::freeNotPrimaryAndReadyObject(const Ray::ObjectKey& key, Ray::ObjectTableManager::ObjectInfo& info){
    if(!info.isPrimary_&&info.status_ == Ray::ObjectTableManager::ObjectInfo::READY&&info.usingCount_==0){//这里要做修改
        if(info.objSize_==0){
            return;
        }
        info.status_=Ray::ObjectTableManager::ObjectInfo::READABLE;
        removeObject(info.objSize_);
        objTable_->deleteKey(key);
    }
}

void Ray::SharedObjectStorage::freeisPrimaryAndReadyObject(const Ray::ObjectKey& key, Ray::ObjectTableManager::ObjectInfo& info){
    if(info.isPrimary_&&info.status_==Ray::ObjectTableManager::ObjectInfo::READY&&info.usingCount_==0){
        if(info.objSize_==0){
            return;
        }
        info.status_=Ray::ObjectTableManager::ObjectInfo::SWAPEDOUT;
        std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
        node->submitTask(DiskIOTask::WRITE,info.objSize_,nullptr);
        freeMemorySize_+=info.objSize_;
    }
}

void Ray::SharedObjectStorage::reclaim(size_t target){
    //首先会选择非primary副本，以及对象在本地
    objTable_->forEach([target,this](const Ray::ObjectKey& key, Ray::ObjectTableManager::ObjectInfo& info)->bool{
        this->freeNotPrimaryAndReadyObject(key,info);
        if(target >freeMemorySize_){
            return true;
        }
        else{
            return false;
        }
    });
    if(freeMemorySize_<target){
        auto freePrimary=[target,this](const Ray::ObjectKey& key, Ray::ObjectTableManager::ObjectInfo& info)->bool{
            this->freeisPrimaryAndReadyObject(key,info);
            if(target >freeMemorySize_){
                return true;
            }
            else{
                return false;
            }
        };
        objTable_->forEach(std::bind(freePrimary,std::placeholders::_1,std::placeholders::_2));
    }
}

void Ray::SharedObjectStorage::setFreeMemorySize(size_t freeMemorySize){
    freeMemorySize_=freeMemorySize;
    //加上一个node->alloc(freeMemorySize);
    std::shared_ptr<Node> node=NodeManager::getIntance().searchNode(localNodeID_);
    node->allocMemory(freeMemorySize);
}