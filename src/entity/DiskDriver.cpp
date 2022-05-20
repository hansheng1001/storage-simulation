#include "DiskDriver.h"
#include "DiskDriverImpl.h"

DiskDriver::DiskDriver(const Disk& disk, DiskIOSchedulePolicy policy)
{
    pImpl_ = std::unique_ptr<DiskDriverImpl>(new DiskDriverImpl(disk, std::move(policy)));
}

DiskDriver::~DiskDriver() = default;

void DiskDriver::submitDiskIOTask(const DiskIOTask& task, CallBack callback)
{
    pImpl_->submitDiskIOTask(task, std::move(callback));
}

const DiskRunTimeState& DiskDriver::getState() const
{
    return pImpl_->getState();
}