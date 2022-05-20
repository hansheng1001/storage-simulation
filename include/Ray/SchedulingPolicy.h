#pragma once
#include "RayBaseDefination.h"
#include <unordered_map>
#include <vector>

namespace Ray {

enum class TaskStatus { ACCEPT, DENY, PULL_FINISH, DONE };

struct NodeFailures {
    size_t failures_;
    Time   nextTime_;
};

struct TaskInfo {
    RayTaskHandle                      taskHandle_;
    std::unordered_map<NodeID, size_t> History_;
    Time                               acceptTime_;
};

class RayScheduler;
class SchedulingPolicy {
    const NodeID                             localNodeID_;
    std::shared_ptr<RayScheduler>            pScheduler_;
    std::unordered_map<size_t, TaskInfo>     taskHistory_;
    std::unordered_map<NodeID, NodeFailures> nodeHistory_;

    Time averageCompletionTime_;

    void updateNodeHistory(const size_t& taskID, const NodeID& nodeID);

public:
    SchedulingPolicy(const NodeID& localNodeID) : localNodeID_(localNodeID) {}
    ~SchedulingPolicy() {}
    // 获取最佳调度节点
    NodeID getBestNodeToSchedule(RayTaskHandle task);
    // 汇报任务时间点信息
    void updateTaskStatus(const size_t& taskID, const NodeID& nodeID, const TaskStatus& status, const Time& time);
};
};  // namespace Ray