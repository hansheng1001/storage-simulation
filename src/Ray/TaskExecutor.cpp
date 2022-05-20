#include "TaskExecutor.h"
#include "../EventLoop.h"
#include "RayAPI.h"
#include "AccountTaskInfo.h"

void Ray::TaskExecutor::tryExecute(Ray::RayTaskHandle task) {
    if (allocResourceCallback_(task->resourceRequest_)) {
        execOneTask(task);
    }
    else {
        pendingTasks_.push_back(task);
    }
}

void Ray::TaskExecutor::execOneTask(Ray::RayTaskHandle task) {
    //从这里发送消息给对应的schedule
    Ray::RayEnvHandle TaskRayEnvHandle = std::make_shared<Ray::RayEnv>();
    TaskRayEnvHandle->service_         = this->service_;
    TaskRayEnvHandle->reportFinish_ = [this, task](ObjectHandle ResultObjectHandle) mutable
    {
        //5.任务执行完成统计
        AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskEndExec, task->taskID_, task->startNodeID_, EventLoop::getInstance().GetCurrentTime());
        this->reduceObjectInTaskUsingCount(task);
        this->whenTaskFinish(task, ResultObjectHandle);
        this->reportTaskExecuateFinish(task->startNodeID_, task);
        this->reportResultCallback_(task->returnValueObjID_, ResultObjectHandle);
    };
    //4.任务开始执行统计
    AccountTaskInfo::getInstance().Account(AccountTaskInfo_ExecStep::TaskStartExec, task->taskID_, task->startNodeID_, EventLoop::getInstance().GetCurrentTime());
    task->taskFunc_(TaskRayEnvHandle);
}

void Ray::TaskExecutor::whenTaskFinish(Ray::RayTaskHandle task, Ray::ObjectHandle result) {
    freeResourceCallback_(task->resourceRequest_);
    tryExecutePendingTasks();
}

void Ray::TaskExecutor::tryExecutePendingTasks() {
    if(pendingTasks_.empty()){
        return;
    }
    for (auto& Task : pendingTasks_) {
        if (allocResourceCallback_(Task->resourceRequest_)) {
            pendingTasks_.remove(Task);
            execOneTask(Task);
        }
    }
}

void Ray::TaskExecutor::submitTask(Ray::RayTaskHandle task) {
    auto WhenReadable = [this, task]() mutable {
        //这里没执行
        this->reportTaskDataReady(task->startNodeID_, task);
        this->tryExecute(task);
    };
    if(task->startNodeID_!=service_->getNodeID()&&task->inputDependency_.size()>0){
        std::shared_ptr<int> number = std::make_shared<int>(task->inputDependency_.size());

        auto whenRaedy=[this,task,WhenReadable,number](){
            (*number)--;
            if(*number==0){
                this->waitInputObjectCallback_(task->inputDependency_, WhenReadable);
            }
        };
        TaskDependencyManager::execWhenDependencyReady(service_->getObjTableManager(),task->inputDependency_, whenRaedy);
    }
    else{
        waitInputObjectCallback_(task->inputDependency_, WhenReadable);
    }
    
}

void Ray::TaskExecutor::exit(){
    pendingTasks_.clear();
}