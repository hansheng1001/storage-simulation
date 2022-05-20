#include "TaskDriver.h"
#include "../include/EventLoop.h"

extern bool iscreateTask;

bool BasicTaskGenerator::genTask(NodeID& id, unsigned& tasktype)
{
    bool res = true;

    if(curRoundBackgroundTasks && curRoundForegroundTasks)
    {
        if(rand() & 1)
        {
            tasktype = rand() % foregroundTaskTypes_;
            --curRoundForegroundTasks;
        }
        else
        {
            tasktype = foregroundTaskTypes_ + rand() % backgroundTaskTypes_;
            --curRoundBackgroundTasks;
        }
    }
    else if(curRoundForegroundTasks)
    {
        tasktype = rand() % foregroundTaskTypes_;
        --curRoundForegroundTasks;
    }
    else if(curRoundBackgroundTasks)
    {
        tasktype = foregroundTaskTypes_ + rand() % backgroundTaskTypes_;
        --curRoundBackgroundTasks;
    }
    else
        res = false;

    if(res)
    {
        //最后生成[1,maxNodeID_]范围的ID号
        id = rand() % maxNodeID_ + 1;
        return res;
    }
    
    curRoundForegroundTasks = foregroundTasksPerPeriod_;
    if((rand() % 100) / 100.0 < burstSubmitRatio_)
        curRoundBackgroundTasks = burstSumitFactor_ * backgroundTasksPerPeriod_;
    else
        curRoundBackgroundTasks = backgroundTasksPerPeriod_;

    return false;
}

void TaskDriver::genTasks()
{
    NodeID id;
    unsigned taskType;

    if(finishTime_ <= EventLoop::getInstance().GetCurrentTime()){
        //设置没有任务产生的标志
        iscreateTask=false;
        return;
    }

    while(genTask_(id, taskType))
        submit_(id, taskType);

    if(EventLoop::getInstance().GetCurrentTime() + genTaskPeriod_ >= finishTime_)
    {
        iscreateTask = false;
        return;
    }
    
    EventLoop::getInstance().callAfter(genTaskPeriod_, [this](){
        genTasks();
    });
}

void TaskDriver::start()
{
    EventLoop::getInstance().callAfter(0, [this](){
        genTasks();
    });
}