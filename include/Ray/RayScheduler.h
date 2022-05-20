#pragma once
#include "Callbacks.h"
#include "Connection.h"
#include "ObjectTableManager.h"
#include "RayBaseDefination.h"
#include "SchedulingPolicy.h"
#include "GlobalInformationClient.h"

namespace Ray {
class ObjectTableManager;
class RayScheduler {
public:
    using NewTaskCallback = std::function<void(Ray::RayTaskHandle)>;

    //test代码
    using TestGlobalData = std::function<void(std::unordered_map<NodeID, Ray::LocalResourceTable> map)>;
    using TestPerData = std::function<void(std::unordered_map<uint32_t, float>)>;
    struct ConnectionsClient_{
        NodeID                  id;
        Connection              connection;
        bool                    isConnect;
        std::vector<CallBack>   calls; 
    };
private:
    NodeID              localNodeID_;
    NewTaskCallback     whenAcceptNewTaskFromRemoteScheduler_;  //绑定的是RayService的acceptTask
    ObjectTableManager* localObjTableManager_;
    std::unordered_map<NodeID, ConnectionsClient_>               connectionsClient_;
    //std::unordered_map<NodeID, std::pair<Connection, bool>> 
    std::shared_ptr<SchedulingPolicy>                       schedulingPolicy_;

    void handleRemoteResponse(bool permitted, RayTaskHandle task);  // task参数用于失败时的重新调度
    void sendRequestToRemoteScheduler(NodeID nodeID, RayTaskHandle task);
   
    void sendTaskScheduleResult(NodeID nodeID, RayTaskHandle task, bool permitted);
    void handleRemoteRequest(NodeID peerNodeID, RayTaskHandle task);  //处理远端请求
    void acceptNewTaskFromRemoteScheduler(RayTaskHandle task) {
        whenAcceptNewTaskFromRemoteScheduler_(task);
    }

    //要告诉远端节点是否能够调度
    Time getTime();

public:
    //test代码
    TestGlobalData  testGlobal_;
    TestPerData     testPer_;


    RayScheduler(NodeID localNodeID);
    void submitTask(RayTaskHandle task);
    void execWhenAcceptNewTaskFromRemote(NewTaskCallback callback);

    void sendTaskDataReadyToRemoteScheduler(NodeID nodeID, RayTaskHandle task);
    void sendTaskExecuateFinish(NodeID nodeID, RayTaskHandle task);

    bool allocResource(const ResourceSet& resourceReques);
    void freeResource(const ResourceSet& resourceToFree);

    void startClient(NodeID nodeID, CallBack call);
    void startServer();
    void push(Connection connect) {
        connectionsClient_[connect.m_destID].isConnect=true;
        connectionsClient_[connect.m_destID].connection=connect;
        connectionsClient_[connect.m_destID].id=connect.m_destID;
        for(auto begin=connectionsClient_[connect.m_destID].calls.begin();begin!=connectionsClient_[connect.m_destID].calls.end();begin++){
            (*begin)();
        }
        connectionsClient_[connect.m_destID].calls.clear();
    }
	
    const LocalResourceTable&                  getLocalResourceInfo() const;
    void  setLocalResourceTable(LocalResourceTable table){
        localResourceTable_=table;
    }
	
    std::shared_ptr<GlobalResourceTable> getGlobalResourceInfo();

    void setGlobalInformationClientCallBack(std::shared_ptr<GlobalInformationClient> infoClient){

		infoClient->setUpdateGlobalResourceCallback([this](std::shared_ptr<GlobalResourceTable> table){
            this->updateGlobalResourceInfo(table);
        });
		
        infoClient->setGetLocalResourceCallback([this]() -> const Ray::LocalResourceTable& { return this->getLocalResourceInfo(); });
    }
	
    void setObjectTableManager(Ray::ObjectTableManager* table){
        localObjTableManager_=table;
    }
	
    void setSchedulePolicy();
    void setTable_(LocalResourceTable table){
        table_=table;
    }
    
    
    //测试代码
    void testGlobal(){
        testGlobal_(*pGlobalResourceTable_);
    }

    void testLocal(){
        testPer_(localResourceTable_);
    }
	
    void exit();

    
private:
    LocalResourceTable                   localResourceTable_;
    LocalResourceTable                   table_;                        
    std::shared_ptr<GlobalResourceTable> pGlobalResourceTable_;
    void                                 TaskHandleMessage(Message message);
    //这里主要是调度策略插件

    //获取本地资源信息的函数，GlobalInformationClient定期调用

    // GlobalInformationClient使用回调此函数报告全局资源信息
    void updateGlobalResourceInfo(std::shared_ptr<GlobalResourceTable> table) {
        pGlobalResourceTable_ = table;
        //testGlobal();
    }

    //比较任务所需资源能不能被本地资源所满足
    bool isLocalResourceSatisfied(const ResourceSet& resourceRequest);
};

}  // namespace Ray
