#pragma once
#include "Computation.h"

//维护CPU内存运行时候的动态信息
class ComputationTaskSchedulerImpl
{
    friend ComputationTaskScheduler;

    ComputationTaskSchedulerImpl(const ComputationArch& arch, ComputationTaskSchedulePolicy policy);

public:
    const ComputationArch& arch_;
    ComputationArchRuntimeState runTimeState_;
    ComputationTaskSchedulePolicy policy_;

    //helper function for policy
    unsigned long getVirtualInstructionNum(const ComputationTask& task, unsigned memoryNodeIndex, unsigned CPUIndex) const;
    unsigned long getVirtualInstructionNum(unsigned long instructionNum, double memoryInstructionRatio, unsigned memoryNodeIndex, unsigned CPUIndex) const;
    Time getExeTime(unsigned long virtualInstructionNum, unsigned CPUIndex);

    void submitCompuationTask(const ComputationTask& task, CallBack callback);
    const ComputationArchRuntimeState& getState() const;
    bool allocMemorySize(size_t size,int number=0);
};