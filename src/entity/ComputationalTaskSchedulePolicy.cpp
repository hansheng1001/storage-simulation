#include "ComputaionalTaskSchedulePolicy.h"
#include "EventLoop.h"

void FIFOComputationalTaskSchedPolicy::whenTaskCompleted(const TaskComplition &comp, ComputationTaskSchedulerImpl &scheduler)
{
    scheduler.runTimeState_.cpuStates_[comp.chosedCPU_].taskFinish();
    scheduler.runTimeState_.nodesMemoryLeft_[comp.chosedMemoryNode_] += comp.memoryConsumed_;

    //async call comp.callback_
    EventLoop::getInstance().addCallBack(comp.callback_);

    pumpPendingTaskQueue(scheduler);
}

void FIFOComputationalTaskSchedPolicy::pumpPendingTaskQueue(ComputationTaskSchedulerImpl& scheduler)
{
    while(!pendingTask_.empty())
    {
        TaskQueueItem& item = pendingTask_.front();
        if(scheduleOneTask(scheduler, item.task_, item.callback_))
            pendingTask_.pop_front();
        else
            return;
    }
}

bool FIFOComputationalTaskSchedPolicy::scheduleOneTask(ComputationTaskSchedulerImpl& scheduler, const ComputationTask& task, CallBack& callback)
{
    unsigned chosedCPU = 0;
    unsigned chosedMemoryNode = 0;

    auto socketNum = scheduler.arch_.sockets_.size();
    auto nodeNum = scheduler.arch_.memoryNodes_.size();

    while (chosedMemoryNode < nodeNum)
    {
        if (scheduler.runTimeState_.nodesMemoryLeft_[chosedMemoryNode] >= task.memoryRequired_)
            break;
        ++chosedMemoryNode;
    }

    if (chosedMemoryNode == nodeNum)
        return false;

    while (chosedCPU < socketNum)
    {
        if (scheduler.runTimeState_.cpuStates_[chosedCPU].hasIdleCore())
            break;
        ++chosedCPU;
    }

    if (chosedCPU == socketNum)
        return false;

    unsigned long virtualInstructionNum = scheduler.getVirtualInstructionNum(task, chosedMemoryNode, chosedCPU);
    Time exeTime = scheduler.getExeTime(virtualInstructionNum, chosedCPU);

    scheduler.runTimeState_.cpuStates_[chosedCPU].assignTask();
    scheduler.runTimeState_.nodesMemoryLeft_[chosedMemoryNode] -= task.memoryRequired_;

    TaskComplition comp{chosedCPU, chosedMemoryNode, task.memoryRequired_, std::move(callback)};
    EventLoop::getInstance().callAfter(exeTime, [this, &scheduler, cp = std::move(comp)](){
        whenTaskCompleted(cp, scheduler);
    });

    return true;
}

void FIFOComputationalTaskSchedPolicy::operator()(ComputationTaskSchedulerImpl &scheduler, const ComputationTask &task, CallBack callback)
{
    if(pendingTask_.empty())
        if(scheduleOneTask(scheduler, task, callback))
            return;

    pendingTask_.emplace_back(task, std::move(callback));
}

bool RRComputationalTaskSchedPolicy::scheduleOneTask(IterType taskIter, ComputationTaskSchedulerImpl& scheduler)
{
    unsigned chosedCPU = 0;
    unsigned chosedMemoryNode = 0;
    TaskToSchedule& task = *taskIter;

    auto socketNum = scheduler.arch_.sockets_.size();
    auto nodeNum = scheduler.arch_.memoryNodes_.size();

    while (chosedMemoryNode < nodeNum)
    {
        if (scheduler.runTimeState_.nodesMemoryLeft_[chosedMemoryNode] >= task.memoryConsumed_)
            break;
        ++chosedMemoryNode;
    }

    if (chosedMemoryNode == nodeNum)
        return false;

    while (chosedCPU < socketNum)
    {
        if (scheduler.runTimeState_.cpuStates_[chosedCPU].hasIdleCore())
            break;
        ++chosedCPU;
    }

    if (chosedCPU == socketNum)
        return false;

    Time exeTime;
    unsigned long insExecuted;
    unsigned long virtualInstructionsLeft = scheduler.getVirtualInstructionNum(task.instructionNum_, task.memoryInstructionRatio_, chosedMemoryNode, chosedCPU);

    if(virtualInstructionExecutedPerSlice_.empty())
        init(scheduler.arch_.sockets_);

    if(virtualInstructionsLeft >= virtualInstructionExecutedPerSlice_[chosedCPU])
    {
        exeTime = timeSliceLength_;
        insExecuted = virtualInstructionExecutedPerSlice_[chosedCPU] * ((double)task.instructionNum_ / virtualInstructionsLeft);
    }
    else
    {
        exeTime = scheduler.getExeTime(virtualInstructionsLeft, chosedCPU);
        insExecuted = task.instructionNum_;
    }

    scheduler.runTimeState_.cpuStates_[chosedCPU].assignTask();
    scheduler.runTimeState_.nodesMemoryLeft_[chosedMemoryNode] -= task.memoryConsumed_;
    taskIter->state = Running;

    EventLoop::getInstance().callAfter(exeTime, [this, &scheduler,
    cp = TaskTimeSliceComplition{chosedCPU, chosedMemoryNode, insExecuted, taskIter}](){
        whenTimeSliceFinished(scheduler, cp);
    });

    return true;
}

void RRComputationalTaskSchedPolicy::trySchedule(ComputationTaskSchedulerImpl& scheduler)
{
    if(taskList_.empty())
        return;

    IterType start = nextTaskToSchedule_;

    do
    {
        if(start->state == Running)
        {
            ++start;
            if(start == taskList_.end())
                start = taskList_.begin();
            continue;
        }

        if(!scheduleOneTask(start, scheduler))
            break;

        ++start;

        if(start == taskList_.end())
            start = taskList_.begin();
    } while (start != nextTaskToSchedule_);
    
    nextTaskToSchedule_ = start;
}

void RRComputationalTaskSchedPolicy::whenTimeSliceFinished(ComputationTaskSchedulerImpl& scheduler, const TaskTimeSliceComplition& comp)
{
    scheduler.runTimeState_.cpuStates_[comp.chosedCPU_].taskFinish();
    scheduler.runTimeState_.nodesMemoryLeft_[comp.chosedMemoryNode_] += comp.taskIter_->memoryConsumed_;

    comp.taskIter_->instructionNum_ -= comp.instructionNumExecuted_;
    comp.taskIter_->state = Ready;

    if(comp.taskIter_->instructionNum_ == 0)
    {
        EventLoop::getInstance().addCallBack(comp.taskIter_->callback_);
        if(comp.taskIter_ == nextTaskToSchedule_)
        {
            nextTaskToSchedule_ = taskList_.erase(comp.taskIter_);
            if(nextTaskToSchedule_ == taskList_.end())
                nextTaskToSchedule_ = taskList_.begin();
        }
        else
            taskList_.erase(comp.taskIter_);                     
    }

    if(!taskList_.empty())
        trySchedule(scheduler);   
}

void RRComputationalTaskSchedPolicy::init(const vector<CPU>& cpus)
{
    for(const CPU& cpu : cpus)
    {
        unsigned long vinsPerSlice = timeSliceLength_ / (cpu.cycle_ * cpu.CPI_);
        virtualInstructionExecutedPerSlice_.push_back(vinsPerSlice);
    }
}

void RRComputationalTaskSchedPolicy::operator()(ComputationTaskSchedulerImpl& scheduler, const ComputationTask& task, CallBack callback)
{
    TaskToSchedule newTask{
        task.instructionNum_,
        task.memoryRequired_,
        task.memoryInstructionRatio_,
        Ready,
        std::move(callback)
    };

    if(taskList_.empty())
    {
        taskList_.push_back(std::move(newTask));
        nextTaskToSchedule_ = taskList_.begin();
    }
    else
        taskList_.push_back(std::move(newTask));


    trySchedule(scheduler);   
}

RRComputationalTaskSchedPolicy::RRComputationalTaskSchedPolicy(unsigned long timeSliceLength):
    timeSliceLength_(timeSliceLength)
{
    nextTaskToSchedule_ = taskList_.end();
}