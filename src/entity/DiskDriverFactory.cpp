#include "DiskDriverFactory.h"
#include "DiskIOSchedulePolicy.h"

std::unique_ptr<DiskDriver> CreateFIFODiskDriver(const Disk& disk, bool fairBandwidth)
{
    return std::make_unique<DiskDriver>(disk, FIFODisKIOSchedulePolicy(fairBandwidth));
}