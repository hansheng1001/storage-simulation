#pragma once
#include "RayScheduler.h"
#include "TaskDependencyManager.h"
#include <type_traits>

#include "ObjectTableManager.h"
#include "TaskExecutor.h"
#include "GlobalInformationClient.h"

#include "AccountTaskInfo.h"

namespace Ray {
class RayService {
    NodeID                              localNodeID_;
    std::unique_ptr<RayScheduler>       localScheduler_;
    std::unique_ptr<ObjectTableManager> localObjTableManager_;
    std::unique_ptr<TaskExecutor>       localWorker_;
    size_t                              taskStartID_;

    void reportResult(ObjectID, ObjectHandle);  //将ObjectID所对应的所有回调函数执行
    void acceptTask(RayTaskHandle task);  //绑定RayScheduler::acceptNewTaskFromRemoteScheduler，这里需要创建一个RayEnv，将其自身的RayService指针和它的
    //如何将RayEnv给赋值了，在这里将*this，并且将task当中的函数再一次打包给RayEnv::reportFinish_,这里假设的是有值
    //[this,localWorker_](task->returnValueObjID_.getObj()){
    //    RayService->reportResult();
    //    TaskExecutor::whenTaskFinish()
    //};
public:
    RayService(NodeID localNodeID) : localNodeID_(localNodeID), taskStartID_(0) {}
    
    /*template <typename TaskFuncType, typename... Params>
    auto remote(TaskFuncType taskfunc, Params &&...params)
        -> typename std::enable_if<std::is_void<decltype(taskfunc(params...))>::value, void>::type
    {
    }

    template <typename TaskFuncType, typename... Params>
    auto remote(TaskFuncType taskfunc, Params &&...params)
        -> typename std::enable_if<std::is_void<decltype(taskfunc(params...))>::value == false, ObjectID>::type
    {
    }*/

    template <typename TaskFuncType, typename... Params>
    auto remote(const ResourceSet& resourceRequest, TaskFuncType taskfunc, Params&&... params) ->
        typename std::enable_if<std::is_void<decltype(taskfunc(RayEnvHandle(), params...))>::value, ObjectID>::type{

		ObjectID id = localObjTableManager_->put(nullptr);
        
        RayTaskHandle task=std::make_shared<Ray::RayTask>();
        task->returnValueObjID_ = id;
        task->startNodeID_      = localNodeID_;
        task->taskID_           = taskStartID_++;
        task->resourceRequest_ = resourceRequest;

        //1.任务提交统计
        AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskSubmit, task->taskID_, localNodeID_, EventLoop::getInstance().GetCurrentTime());
        

        auto taskFunc          = [taskfunc, params...](RayEnvHandle handle)mutable{ 
            taskfunc(handle, params...); 
        };
		
        task->taskFunc_         = std::bind(taskFunc, std::placeholders::_1);


        std::vector<ObjectID> dependencyList;
        TaskDependencyManager::gatherInputDependency(dependencyList, params...);
        task->inputDependency_.swap(dependencyList);

        int relyNumber = task->inputDependency_.size();
		
        //如果没有依赖就可以直接提交执行
        std::shared_ptr<int> number = std::make_shared<int>(relyNumber);
        if (relyNumber == 0) {
            //2.任务资源满足统计
            AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskResourceSatisfy, task->taskID_, localNodeID_, EventLoop::getInstance().GetCurrentTime());
            localScheduler_->submitTask(task);
        }
        else {
            auto whenRaedy = [number, this, task]() mutable {
                (*number)--;
                if (*number == 0)
                {
                    //2.任务资源满足统计
                    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskResourceSatisfy, task->taskID_, localNodeID_, EventLoop::getInstance().GetCurrentTime());
                    localScheduler_->submitTask(task);
                }
            };
            ObjectTableManager* localObjTableManager=localObjTableManager_.get();
            TaskDependencyManager::execWhenDependencyReady(localObjTableManager,task->inputDependency_, whenRaedy);
            
        }

        // 1.定义任务RayTask
        // 2.任务依赖
        // 1.1 gatherInputDependency
        // 1.2 execWhenDependencyReady 这个只是确定ObjectInfo所对应的Object已经产生
        // 3.调用submitTask()
        // 4.解析全局资源选择合适的节点来调度pGlobalResourceTable_
        // 5.向对应的节点发送消息sendRequestToRemoteScheduler,handleRemoteResponse
        // 6.对应的节点handleRemoteRequest
        // 7.判断任务的依赖是否完成acceptNewTaskFromRemoteScheduler
        // 8.如果内存不够就会进行替换
        return id;
    }

    /*template <typename TaskFuncType, typename... Params>
    auto remote(const std::vector<ResourceRequirement> &resourceRequirements, TaskFuncType taskfunc, Params &&...params)
        -> typename std::enable_if<std::is_void<decltype(taskfunc(params...))>::value == false, void>::type
    {
    }*/

    NodeID getNodeID() const;

    //参考Ray的API
    ObjectID put(ObjectHandle obj);
    void deleteObj(ObjectKey& key);
    ObjectHandle getObj(ObjectID objID);
    ObjectTableManager* getObjTableManager();

    void setPtr(NodeID id);
    void setInfomationClientCallBack(std::shared_ptr<GlobalInformationClient> client);
    void setLocalResourceTable(LocalResourceTable table);
    void exit();
};
};  // namespace Ray