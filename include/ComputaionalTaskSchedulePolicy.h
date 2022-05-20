#pragma once

#include <list>
#include "ComputationImpl.h"

using std::list;

class FIFOComputationalTaskSchedPolicy
{
    struct TaskQueueItem
    {
        ComputationTask task_;
        CallBack callback_;

        template<typename CallBackType>
        TaskQueueItem(const ComputationTask& task, CallBackType&& callback):
            task_(task), callback_(std::forward<CallBackType>(callback)){}
    };

    struct TaskComplition
    {
        unsigned chosedCPU_;
        unsigned chosedMemoryNode_;
        unsigned long memoryConsumed_;
        CallBack callback_;
    };
    
    list<TaskQueueItem> pendingTask_;

    //must ensure scheduler is alive when this function is called
    void whenTaskCompleted(const TaskComplition& comp, ComputationTaskSchedulerImpl& scheduler);
    bool scheduleOneTask(ComputationTaskSchedulerImpl& scheduler, const ComputationTask& task, CallBack& callback);
    void pumpPendingTaskQueue(ComputationTaskSchedulerImpl& scheduler);

public:
    void operator()(ComputationTaskSchedulerImpl& scheduler, const ComputationTask& task, CallBack callback);
};

class RRComputationalTaskSchedPolicy
{
    enum TaskState {
        Ready,
        Running
    };

    struct TaskToSchedule
    {
        unsigned long instructionNum_;
        unsigned long memoryConsumed_;
        double memoryInstructionRatio_;
        TaskState state;
        CallBack callback_;
    };

    list<TaskToSchedule> taskList_;

    using IterType = list<TaskToSchedule>::iterator;

    struct TaskTimeSliceComplition
    {
        unsigned chosedCPU_;
        unsigned chosedMemoryNode_;
        unsigned long instructionNumExecuted_;
        IterType taskIter_;
    };

    IterType nextTaskToSchedule_;
    unsigned long timeSliceLength_;
    vector<unsigned long> virtualInstructionExecutedPerSlice_;

    bool scheduleOneTask(IterType taskIter, ComputationTaskSchedulerImpl& scheduler);
    void trySchedule(ComputationTaskSchedulerImpl& scheduler);

    void whenTimeSliceFinished(ComputationTaskSchedulerImpl& scheduler, const TaskTimeSliceComplition& comp);

    void init(const vector<CPU>& cpus);

public:
    RRComputationalTaskSchedPolicy(unsigned long timeSliceLength);  

    void operator()(ComputationTaskSchedulerImpl& scheduler, const ComputationTask& task, CallBack callback);
};