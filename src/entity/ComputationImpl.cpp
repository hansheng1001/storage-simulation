#include "ComputationImpl.h"

ComputationTaskSchedulerImpl::ComputationTaskSchedulerImpl(const ComputationArch& arch, ComputationTaskSchedulePolicy policy):
    arch_(arch), policy_(policy)
{
    for(const CPU& cpu : arch_.sockets_)
        runTimeState_.cpuStates_.emplace_back(cpu.coreNum_);
    for(const Memory& mem : arch_.memoryNodes_)
        runTimeState_.nodesMemoryLeft_.push_back(mem.size_);
}

unsigned long ComputationTaskSchedulerImpl::getVirtualInstructionNum(const ComputationTask& task, unsigned memoryNodeIndex, unsigned CPUIndex) const
{
    double factor = arch_.memoryNodes_[memoryNodeIndex].latencyFactor_;
    if(arch_.sockets_[CPUIndex].node_ != memoryNodeIndex)
        factor *= arch_.NUMALatencyFactor_;

    return task.instructionNum_ + (unsigned long)(task.instructionNum_ * task.memoryInstructionRatio_ * (factor - 1));
}

unsigned long ComputationTaskSchedulerImpl::getVirtualInstructionNum(unsigned long instructionNum, double memoryInstructionRatio, unsigned memoryNodeIndex, unsigned CPUIndex) const
{
    double factor = arch_.memoryNodes_[memoryNodeIndex].latencyFactor_;
    if(arch_.sockets_[CPUIndex].node_ != memoryNodeIndex)
        factor *= arch_.NUMALatencyFactor_;

    return instructionNum + (unsigned long)(instructionNum * memoryInstructionRatio * (factor - 1));
}

Time ComputationTaskSchedulerImpl::getExeTime(unsigned long virtualInstructionNum, unsigned CPUIndex)
{
    unsigned long clock = virtualInstructionNum * arch_.sockets_[CPUIndex].CPI_;
    return clock * arch_.sockets_[CPUIndex].cycle_;
}

void ComputationTaskSchedulerImpl::submitCompuationTask(const ComputationTask& task, CallBack callback)
{
    policy_(*this, task, callback);
}

const ComputationArchRuntimeState& ComputationTaskSchedulerImpl::getState() const
{
    return runTimeState_;
}

bool ComputationTaskSchedulerImpl::allocMemorySize(size_t size,int number){
    if(size>arch_.memoryNodes_[number].size_){
        return false;
    }
    else{
        size_t leftmemory=arch_.memoryNodes_[number].size_-size;
        runTimeState_.nodesMemoryLeft_[number]=leftmemory;
        return true;
    }
}