#pragma once

#include <vector>

struct DiskConfigStore
{
    unsigned ChannelNum_;

    //bytes per second
    unsigned long MaxReadBandwidth_;
    unsigned long MaxWriteBandWidth_;
    unsigned long MaxReadBandWidthPerChannel_;
    unsigned long MaxWriteBandWidthPerChannel_;

    unsigned long ReadBaseLatency_;
    unsigned long WriteBaseLatency_;
};

struct DiskConfigStoreBuffer
{
    std::vector<DiskConfigStore> Disks_;
};