#include "Computation.h"
#include "ComputationImpl.h"

ComputationTaskScheduler::ComputationTaskScheduler(const ComputationArch& arch, ComputationTaskSchedulePolicy policy)
{
    pImpl_ = std::unique_ptr<ComputationTaskSchedulerImpl>(new ComputationTaskSchedulerImpl(arch, policy));
}

void ComputationTaskScheduler::submitCompuationTask(const ComputationTask& task, CallBack callback)
{
    pImpl_->submitCompuationTask(task, callback);
}

const ComputationArchRuntimeState & ComputationTaskScheduler::getState() const
{
    return pImpl_->getState();
}

bool ComputationTaskScheduler::allocSharedMemory(size_t size){
    return pImpl_->allocMemorySize(size);
}

ComputationTaskScheduler::~ComputationTaskScheduler() = default;