#include "ComputationTaskSchedulerFactory.h"
#include "ComputaionalTaskSchedulePolicy.h"

std::unique_ptr<ComputationTaskScheduler> CreateFIFOComputationTaskScheduler(const ComputationArch& arch)
{
    return std::make_unique<ComputationTaskScheduler>(arch, FIFOComputationalTaskSchedPolicy());
}

std::unique_ptr<ComputationTaskScheduler> CreateRRComputationTaskScheduler(const ComputationArch& arch, unsigned long timeSliceLen)
{
    return std::make_unique<ComputationTaskScheduler>(arch, RRComputationalTaskSchedPolicy(timeSliceLen));
}