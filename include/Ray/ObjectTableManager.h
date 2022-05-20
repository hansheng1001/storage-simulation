#pragma once
#include "Callbacks.h"
#include "Connection.h"
#include "RayBaseDefination.h"
#include <list>
#include <unordered_map>


namespace Ray {
class SharedObjectStorage;

using DependencyCallback = std::function<void()>;
using ReadyCallback      = std::function<void()>;

class ObjectTableManager {
public:
    struct ObjectInfo {
        enum ObjStatus {
            PENDING,   //对象数据未产生  这个不改变
            READABLE,  //对象数据已产生但不在本地 这个也不改变
            READY,     //对象数据立即可用 这个要改成READABLE
            SWAPEDOUT  //对象数据被换出至磁盘 这个要改成READABLE
        };

        size_t    objSize_;
        ObjStatus status_;
        uint32_t usingCount_;  //该对象被处于等待或者执行中的任务引用的次数，也就是必须放在内存当中
        bool                          isPrimary_;
        std::list<DependencyCallback> DependencyCallbackQueue_;
        std::list<ReadyCallback>      ReadyCallbackQueue_;
        std::vector<Ray::RayService*> copies_;  //所有可能拥有该对象副本的节点

        ObjectHandle obj_;
    };
	
    struct ConnectionsClient_{
        NodeID                  id;
        Connection              connection;
        bool                    isConnect;
        std::vector<CallBack>   calls; 
    };
    

    ObjectInfo INVALID_OBIECTINFOMATION = {0,Ray::ObjectTableManager::ObjectInfo::SWAPEDOUT,0,false};
	
private:
    std::unordered_map<ObjectKey, ObjectInfo, ObjectKeyHashFunc> objTable_;
    NodeID                                                       localNodeID_;
    std::unordered_map<NodeID, ConnectionsClient_>               connectionsClient_;


    RayService*                                                  rayService_;//这个要设置
    std::shared_ptr<SharedObjectStorage>                         sharedObjectptr_;//这个要设置
    u_int32_t                                                    startObjectID;

    void sendReadObjectRequest(ObjectID objID);
    void receiveReadObjectRequest(RayService* peerRayService, ObjectID objID);

    void sendReadObjectInfoRequest(ObjectID objID);
    void receiveReadObjectInfoRequest(RayService* peerRayService, ObjectID objID);

    void sendObjectInfo(ObjectID objID, size_t objSize, NodeID nodeid);
    void sendObject(ObjectID objID, ObjectHandle obj, NodeID nodeid);

    void receiveObjectInfo(ObjectID objID, size_t objSize);
    void receiveObject(ObjectID objID, ObjectHandle obj);

    // 这个函数的作用
    void receiveCopyRemovedNotification(RayService* peerRayService, const ObjectKey& key);
    void notifyNodeToRemoveCopy(const ObjectKey& key, NodeID id);

    void sendRemovedNotification(ObjectKey& key, NodeID id);
    void receiveRemovedObjectnotification(ObjectKey& key);

    std::variant<bool, Connection> searchConnection(NodeID);
    void                           sendReadObjectFromOtherNode(ObjectID objID, NodeID id);
    
public:
    
    void AcceptCallBack(Connection connection);
    void ReadCallBack(Message msg);
    ObjectTableManager(NodeID localNodeID) : localNodeID_(localNodeID), startObjectID(0) {}

    void waitUntilReadable(ObjectID obj, DependencyCallback callback);
    void waitUntilReady(ObjectID obj, ReadyCallback callback);

    ObjectHandle get(ObjectID objID);
    ObjectID     put(ObjectHandle obj);
    bool         addObjectToMemory(size_t objSize);

    void reportResult(ObjectID, ObjectHandle);
    void getResult(ObjectID, ObjectHandle);
    void forEach(std::function<bool(const ObjectKey&, ObjectInfo&)>);  //返回的bool是为了不在需要空间?????
    void                           deleteKey(const ObjectKey& key);  //只管自己的对象表
     
    //得到info
    ObjectInfo getConstInfomation(const ObjectKey& key);
    ObjectInfo& getInfomation(const ObjectKey& key);

    void deletObj(ObjectKey& key);

    void push(Connection connect) {
        connectionsClient_[connect.m_destID].isConnect=true;
        connectionsClient_[connect.m_destID].connection=connect;
        connectionsClient_[connect.m_destID].id=connect.m_destID;
        for(auto begin=connectionsClient_[connect.m_destID].calls.begin();begin!=connectionsClient_[connect.m_destID].calls.end();begin++){
            (*begin)();
        }
    }
    const Connection& searchConnection(NodeID, bool&, bool& result);

    void      startClient(NodeID nodeid, CallBack call);
    void      startServer();
    u_int32_t CreateObjectID();

    void HandleMessage(Message message);

    void setRayService(RayService* service){
        rayService_=service;
    }

    void setSharedObjectStorage(const float memoryRatio,float memorySize);
    void reduceObjectUsingCountInRayTask(Ray::RayTaskHandle task);
    void exit();
};
}  // namespace Ray