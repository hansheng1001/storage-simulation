#include "SchedulingPolicy.h"
#include "EventLoop.h"
#include "RayScheduler.h"
#include "stdlib.h"
#include <memory>

const double updateTimeWeight   = 0.5;
const double updateSpeedWeight  = 0.5;
const size_t maxFailures        = 10;
const size_t maxFailuresPerTask = 3;
const Time   timeInBlackList    = 10;

extern int nodeNumber;

NodeID Ray::SchedulingPolicy::getBestNodeToSchedule(RayTaskHandle task) { 
    float r=(rand()%11)/10.0;
    if(r<0.5){
        return task->startNodeID_;
    }
    else{
        return rand() % (nodeNumber-1) + 1;
    }

    // //return task->startNodeID_;

    //return rand() % (nodeNumber-1) + 1;
}

void Ray::SchedulingPolicy::updateTaskStatus(const size_t& taskID, const NodeID& nodeID, const TaskStatus& status,
                                             const Time& time) {
    // switch (status) {
    //     case TaskStatus::ACCEPT:
    //         taskHistory_[taskID].acceptTime_ = time;
    //         break;
    //     case TaskStatus::DENY:
    //         updateNodeHistory(taskID, nodeID);
    //         break;
    //     case TaskStatus::PULL_FINISH:
    //         // 这里我需要一个获取任务参数的大小的函数
    //         break;
    //     case TaskStatus::DONE:
    //         averageCompletionTime_ = (1 - updateTimeWeight) * averageCompletionTime_ + updateTimeWeight * time;
    //         taskHistory_.erase(taskID);
    //         break;
    //     default:
    //         break;
    // }
}

void Ray::SchedulingPolicy::updateNodeHistory(const size_t& taskID, const NodeID& nodeID) {
    if (nodeHistory_.find(nodeID) != nodeHistory_.end()) {
        nodeHistory_[nodeID].failures_++;
    }
    else {
        nodeHistory_[nodeID].failures_ = 1;
    }
    if (nodeHistory_[nodeID].failures_ >= maxFailures) {
        nodeHistory_[nodeID].failures_ = 0;
        nodeHistory_[nodeID].nextTime_ = EventLoop::getInstance().GetCurrentTime() + timeInBlackList;
    }
}