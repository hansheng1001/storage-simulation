#include "DiskDriver.h"
#include <memory>

std::unique_ptr<DiskDriver> CreateFIFODiskDriver(const Disk& disk, bool fairBandwidth);