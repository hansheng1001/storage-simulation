#pragma once

#include <vector>

struct CPUConfigStore
{
    double Cycle_;
    double CPI_;
    unsigned CoreNum_;
    unsigned Node_;
};

struct MemoryConfigStore
{
    unsigned long Size_;
    double LatencyFactor_;
};

struct CPUAndMemoryConfigStoreBuffer
{
    std::vector<CPUConfigStore> Sockets_;
    std::vector<MemoryConfigStore> MemoryNodes_;

    double NUMALatencyFactor_;
};
