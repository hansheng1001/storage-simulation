#pragma once
#include "CPU.h"
#include "Memory.h"
#include "Callbacks.h"
#include "Base.h"

#include <memory>
#include <vector>
#include <functional>

using std::vector;

struct ComputationArch
{
    vector<CPU> sockets_;
    vector<Memory> memoryNodes_;

    // 1 for UMA
    double NUMALatencyFactor_;
};

struct ComputationTask
{
    unsigned long instructionNum_;
    double memoryInstructionRatio_;
    unsigned long memoryRequired_;
    unsigned long userInfo_;
};

struct ComputationArchRuntimeState
{
    vector<CPUStatus> cpuStates_;
    vector<unsigned long> nodesMemoryLeft_;
};

class ComputationTaskSchedulerImpl;
using ComputationTaskSchedulePolicy = std::function<void (ComputationTaskSchedulerImpl&, const ComputationTask&, CallBack)>;

class ComputationTaskScheduler
{
private:
    std::unique_ptr<ComputationTaskSchedulerImpl> pImpl_;
public:
    ComputationTaskScheduler(const ComputationArch& arch, ComputationTaskSchedulePolicy policy);
    ~ComputationTaskScheduler();
    void submitCompuationTask(const ComputationTask& task, CallBack callback);
    bool allocSharedMemory(size_t size);
    const ComputationArchRuntimeState& getState() const;
};