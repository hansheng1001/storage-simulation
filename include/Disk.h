#pragma once
#include "Base.h"

struct Disk
{
    unsigned channelNum_;

    //bytes per second
    unsigned long maxReadBandwidth_;
    unsigned long maxWriteBandWidth_;
    unsigned long maxReadBandWidthPerChannel_;
    unsigned long maxWriteBandWidthPerChannel_;

    Time readBaseLatency_;
    Time writeBaseLatency_;
};