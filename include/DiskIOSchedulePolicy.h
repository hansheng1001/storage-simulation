#pragma once
#include "DiskDriverImpl.h"
#include <list>

class FIFODisKIOSchedulePolicy
{
    struct PendingIO
    {
        DiskIOTask task_;
        CallBack callback_;

        template<typename CallBackType>
        PendingIO(const DiskIOTask& task, CallBackType&& callback):
        task_(task), callback_(std::forward<CallBackType>(callback)){}
    };

    std::list<PendingIO> pendingIOQueue_;
    bool fairBandwidth_;
    unsigned long channelReadAvgBandwidth_;
    unsigned long channelWriteAvgBandwidth_;

    bool tryStartOneTask(DiskDriverImpl& driver, const DiskIOTask& task, CallBack& callback);
    void whenIOFinish(DiskDriverImpl& driver, unsigned long bandwidth, DiskIOTask::Direction direction, CallBack&& cb);
    void pumpPendingIOQueue(DiskDriverImpl& driver);

public:
    FIFODisKIOSchedulePolicy(bool fairBandWidth):
    fairBandwidth_(fairBandWidth), channelReadAvgBandwidth_(0), channelWriteAvgBandwidth_(0){}

    void operator()(DiskDriverImpl& driver, const DiskIOTask& task, CallBack callback);
};