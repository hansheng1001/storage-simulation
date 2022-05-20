#pragma once
#include "Disk.h"
#include "Callbacks.h"
#include <memory>

struct DiskIOTask
{
    enum Direction{
        READ,
        WRITE
    };

    unsigned long IOLength_;
    Direction direction_;
    unsigned long userInfo_;
};

struct DiskRunTimeState
{
    unsigned long readBandWidthLeft_;
    unsigned long writeBandWidthLeft_;
    unsigned channelLeft_;
};

class DiskDriverImpl;

using DiskIOSchedulePolicy = std::function<void (DiskDriverImpl&, const DiskIOTask&, CallBack)>;

class DiskDriver
{
    std::unique_ptr<DiskDriverImpl> pImpl_;

public:
    DiskDriver(const Disk& disk, DiskIOSchedulePolicy policy);
    ~DiskDriver();

    void submitDiskIOTask(const DiskIOTask& task, CallBack callback);
    const DiskRunTimeState& getState() const;
};