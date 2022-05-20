#include "DiskIOSchedulePolicy.h"
#include "EventLoop.h"

bool FIFODisKIOSchedulePolicy::tryStartOneTask(DiskDriverImpl& driver, const DiskIOTask& task, CallBack& callback)
{
    if(!driver.runTimeState_.channelLeft_)
        return false;

    --driver.runTimeState_.channelLeft_;

    if(!channelReadAvgBandwidth_)
    {
        channelReadAvgBandwidth_ = (double)driver.disk_.maxReadBandwidth_ / driver.disk_.channelNum_;
        channelWriteAvgBandwidth_ = (double)driver.disk_.maxWriteBandWidth_ / driver.disk_.channelNum_;
        channelReadAvgBandwidth_ = std::min(channelReadAvgBandwidth_, driver.disk_.maxReadBandWidthPerChannel_);
        channelWriteAvgBandwidth_ = std::min(channelWriteAvgBandwidth_, driver.disk_.maxWriteBandWidthPerChannel_);
    }

    unsigned long bandWidthAlloced;
    Time baseLatency;
    if(task.direction_ == DiskIOTask::Direction::READ)
    {
        if(fairBandwidth_)
            bandWidthAlloced = channelReadAvgBandwidth_;
        else
            bandWidthAlloced = std::min(driver.runTimeState_.readBandWidthLeft_, driver.disk_.maxReadBandwidth_);

        driver.runTimeState_.readBandWidthLeft_ -= bandWidthAlloced;
        baseLatency = driver.disk_.readBaseLatency_;
    }
    else
    {
        if(fairBandwidth_)
            bandWidthAlloced = channelWriteAvgBandwidth_;
        else
            bandWidthAlloced = std::min(driver.runTimeState_.writeBandWidthLeft_, driver.disk_.maxWriteBandWidth_);

        driver.runTimeState_.writeBandWidthLeft_ -= bandWidthAlloced;
        baseLatency = driver.disk_.writeBaseLatency_;
    }

    EventLoop::getInstance().callAfter(driver.getIOTime(task.IOLength_, bandWidthAlloced) + baseLatency,
    [this, &driver, bandWidthAlloced, direction = task.direction_, cb = std::move(callback)]()mutable{
        whenIOFinish(driver, bandWidthAlloced, direction, std::move(cb));
    });

    return true;
}

void FIFODisKIOSchedulePolicy::whenIOFinish(DiskDriverImpl& driver, unsigned long bandwidth, DiskIOTask::Direction direction, CallBack&& cb)
{
    ++driver.runTimeState_.channelLeft_;
    if(direction == DiskIOTask::Direction::READ)
        driver.runTimeState_.readBandWidthLeft_ += bandwidth;
    else
        driver.runTimeState_.writeBandWidthLeft_ += bandwidth;

    EventLoop::getInstance().addCallBack(std::move(cb));

    if(!pendingIOQueue_.empty())
        pumpPendingIOQueue(driver);
}

void FIFODisKIOSchedulePolicy::pumpPendingIOQueue(DiskDriverImpl& driver)
{
    while(!pendingIOQueue_.empty())
    {
        PendingIO& task = pendingIOQueue_.front();
        if(tryStartOneTask(driver, task.task_, task.callback_))
            pendingIOQueue_.pop_front();
        else
            break;
    }
}

void FIFODisKIOSchedulePolicy::operator()(DiskDriverImpl& driver, const DiskIOTask& task, CallBack callback)
{
    if(pendingIOQueue_.empty())
        if(tryStartOneTask(driver, task, callback))
            return;

    pendingIOQueue_.emplace_back(task, std::move(callback));
}