#pragma once


#ifdef TestStorage
    #include"ObjTableManager.h"
    typedef ObjTableManager Manager;
    
#else
    #include "ObjectTableManager.h"
    typedef Ray::ObjectTableManager Manager;
#endif 
    




class Node;
//这里初始化的时候需要先从节点内部预先分配内存
namespace Ray {
class SharedObjectStorage {
    size_t              localSharedMemorySize_;
    size_t              freeMemorySize_;
    Manager*            objTable_;
    NodeID              localNodeID_;
    // Node* node_;//用于将对象写入本地磁盘
 
    //回收至少target字节的内存空间
    void reclaim(size_t target);
public:
    SharedObjectStorage(NodeID id):localNodeID_(id){
    }
    bool addObject(size_t objSize);  //分配空间，也就是说先分配到内存，再到磁盘
    void removeObject(size_t objSize);
    
    
    void setLocalSharedMemorySize(size_t localSharedMemorySize){
        localSharedMemorySize_=localSharedMemorySize;
    }
    void setFreeMemorySize(size_t freeMemorySize);
    void setObjectTableManager(Manager* objTable){
        objTable_=objTable;
    }
    void freeNotPrimaryAndReadyObject(const ObjectKey& key, Ray::ObjectTableManager::ObjectInfo& info);
    void freeisPrimaryAndReadyObject(const ObjectKey& key, Ray::ObjectTableManager::ObjectInfo& info);
};
};  // namespace Ray