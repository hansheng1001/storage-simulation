#pragma once
#include<unordered_map>
#include<memory>
#include "RayBaseDefination.h"
#include "ObjectTableManager.h"

//测试

class ObjTableManager{
    public:
    std::unordered_map<Ray::ObjectKey, Ray::ObjectTableManager::ObjectInfo, Ray::ObjectKeyHashFunc> objTable_;
    std::shared_ptr<Ray::SharedObjectStorage> sharedObjectptr_;//这个要设置
    
    void forEach(std::function<bool (const Ray::ObjectKey&, Ray::ObjectTableManager::ObjectInfo&)> call){
        for(auto begin=objTable_.begin();begin!=objTable_.end();){
            auto temp=begin;
            begin++;
            bool status=call(temp->first,temp->second);
            if(!status){
                return;
            }
        }
    }

    void addObjectToMemory(size_t objSize);
    void deleteKey(const Ray::ObjectKey& key);
    void put(Ray::ObjectHandle obj,size_t using_count,bool isPrimary,int perNodeObjID);
};