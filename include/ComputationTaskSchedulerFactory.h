#include "Computation.h"
#include <memory>

std::unique_ptr<ComputationTaskScheduler> CreateFIFOComputationTaskScheduler(const ComputationArch& arch);
std::unique_ptr<ComputationTaskScheduler> CreateRRComputationTaskScheduler(const ComputationArch& arch, unsigned long timeSliceLen);