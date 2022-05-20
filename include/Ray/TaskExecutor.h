#pragma once
#include "InputObjectReader.h"
#include "RayBaseDefination.h"
#include <list>

namespace Ray {
class TaskExecutor {
    //using WaitObjectCall=std::function<void(const std::vector<ObjectID>&, std::function<void()>)>;

    //这里被执行绑定
    RayService*              service_;
    std::list<RayTaskHandle> pendingTasks_;

    std::function<bool(const ResourceSet&)>                                  allocResourceCallback_;
    std::function<void(const ResourceSet&)>                                  freeResourceCallback_;
    std::function<void(const std::vector<ObjectID>&, std::function<void()>)> waitInputObjectCallback_;
    std::function<void(ObjectID, ObjectHandle)>                              reportResultCallback_;
    std::function<void(NodeID, RayTaskHandle)>                               reportTaskDataReady;  //这里绑定的是Ray::RayScheduler::sendTaskDataReadyToRemoteScheduler
    std::function<void(NodeID, RayTaskHandle)>                               reportTaskExecuateFinish;
    std::function<void(Ray::RayTaskHandle task)>                             reduceObjectInTaskUsingCount;//这个要写

    // 2.在调用allocResourceCallback_去分配资源，
    void tryExecute(RayTaskHandle task);
    void execOneTask(RayTaskHandle task);
	
    //回收资源freeResourceCallback_和tryExecutePendingTasks
    void whenTaskFinish(RayTaskHandle task, ObjectHandle result);
    void tryExecutePendingTasks();

public:
    // 1.调用waitInputObjectCallback_等待所有的资源抵达本地，
    // 2.再调用tryExecute
    TaskExecutor(){}
    TaskExecutor(RayService* service):service_(service){}
	
    void submitTask(RayTaskHandle task);

    void setAllocResourceCallback(std::function<bool(const ResourceSet&)> call){
        allocResourceCallback_=call;
    }
     
    void setFreeResourceCallback(std::function<void(const ResourceSet&)> call){
        freeResourceCallback_=call;
    }

    void setWaitInputObjectCallback(std::function<void(const std::vector<ObjectID>&, std::function<void()>)> call){
        waitInputObjectCallback_=call;
    }

    void setReportResultCallback(std::function<void(ObjectID, ObjectHandle)> call){
        reportResultCallback_=call;
    }

    void setReportTaskDataReady(std::function<void(NodeID, RayTaskHandle)> call){
        reportTaskDataReady=call;
    }

    void setReportTaskExecuateFinish(std::function<void(NodeID, RayTaskHandle)> call){
        reportTaskExecuateFinish=call;
    }

    void setReduceObjectInTaskUsingCount(std::function<void(Ray::RayTaskHandle task)> call){
        reduceObjectInTaskUsingCount=call;
    }
	
    void exit();
};
}  // namespace Ray
