#pragma once

#include "../include/Base.h"
#include <functional>
#include <stdlib.h>
#include <time.h>

using SubmitTaskCallback = std::function<void(NodeID, unsigned)>;
using GenTaskCallback = std::function<bool(NodeID&, unsigned&)>;

class BasicTaskGenerator
{
    //前台任务类型编号一定从0开始
    unsigned foregroundTaskTypes_;//前台任务类型总数
    unsigned backgroundTaskTypes_;//后台任务类型总数
    unsigned backgroundTasksPerPeriod_;//每周期后台任务数量
    unsigned foregroundTasksPerPeriod_;//每周期前台任务数量
    double burstSumitFactor_; //task高峰时，task数量因子
    double burstSubmitRatio_;//task突发高峰界限
    NodeID maxNodeID_; //最后生成[1,maxNodeID_]范围的ID号

    unsigned curRoundForegroundTasks;
    unsigned curRoundBackgroundTasks;

public:
    BasicTaskGenerator(unsigned foregroundTaskTypes, unsigned backgroundTaskTypes,
        unsigned backgroundTaskPerPeriod, unsigned foregroundTasksPerPeriod,
        unsigned burstSumitFactor, double burstSubmitRatio, NodeID maxNodeID) :
        foregroundTaskTypes_(foregroundTaskTypes),
        backgroundTaskTypes_(backgroundTaskTypes),
        backgroundTasksPerPeriod_(backgroundTaskPerPeriod),
        foregroundTasksPerPeriod_(foregroundTasksPerPeriod),
        burstSumitFactor_(burstSumitFactor),
        burstSubmitRatio_(burstSubmitRatio),
        maxNodeID_(maxNodeID)
    {
        //srand(time(NULL));
        curRoundForegroundTasks = foregroundTasksPerPeriod;
        curRoundBackgroundTasks = backgroundTaskPerPeriod;
    }

    bool genTask(NodeID& id, unsigned& type);
};

class TaskDriver
{
    Time finishTime_;
    Time genTaskPeriod_;

    SubmitTaskCallback submit_;
    GenTaskCallback genTask_;

    void genTasks();

public:
    TaskDriver(Time finishTime, Time genTaskPeriod, SubmitTaskCallback submit, GenTaskCallback genTask) :
        finishTime_(finishTime),
        genTaskPeriod_(genTaskPeriod),
        submit_(submit),
        genTask_(genTask)
    { }

    void start();
};