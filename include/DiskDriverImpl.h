#pragma once

#include "DiskDriver.h"

class DiskDriverImpl
{
    friend DiskDriver;

    template<typename PolicyType>
    DiskDriverImpl(const Disk& disk, PolicyType&& policy):
        disk_(disk), policy_(std::forward<PolicyType>(policy))
    {
        runTimeState_.readBandWidthLeft_ = disk_.maxReadBandwidth_;
        runTimeState_.writeBandWidthLeft_ = disk_.maxWriteBandWidth_;
        runTimeState_.channelLeft_ = disk_.channelNum_;
    }

public:
    const Disk& disk_;

    DiskRunTimeState runTimeState_;
    DiskIOSchedulePolicy policy_;

    Time getIOTime(unsigned long bytes, unsigned long bandwidth)
    {
        return (double)bytes / bandwidth * 1e9;       
    }

    template<typename CallBackType>
    void submitDiskIOTask(const DiskIOTask& task, CallBackType&& callback){
        policy_(*this, task, std::forward<CallBackType>(callback));
    }

    const DiskRunTimeState& getState() const{
        return runTimeState_;
    }
};