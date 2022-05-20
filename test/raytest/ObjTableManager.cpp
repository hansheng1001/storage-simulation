#include "ObjTableManager.h"
#include "RayBaseDefination.h"
#include "SharedObjectStorage.h"

void ObjTableManager::deleteKey(const Ray::ObjectKey& key){
}

void ObjTableManager::addObjectToMemory(size_t objSize){
    sharedObjectptr_->addObject(objSize);
}

void ObjTableManager::put(Ray::ObjectHandle obj,size_t using_count,bool isPrimary,int perNodeObjID){
    Ray::ObjectTableManager::ObjectInfo info;
    
    info.isPrimary_=isPrimary;
    info.usingCount_=using_count;
    info.obj_=obj;
    info.objSize_=obj->size_;
    info.status_=Ray::ObjectTableManager::ObjectInfo::READY;
    addObjectToMemory(obj->size_);
    //定义ObjectKey
    u_int32_t id=perNodeObjID;
    Ray::ObjectKey key(1,id);
    objTable_.insert(std::pair<Ray::ObjectKey,Ray::ObjectTableManager::ObjectInfo>(key,info));
    
}