#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <stdarg.h>
#include <EventLoop.h>

enum AccountTaskInfo_ExecStep { TaskSubmit, TaskResourceSatisfy, TaskScheduled, TaskStartExec, TaskEndExec };

class AccountTaskInfo
{
private:
    struct TaskID
    {
        uint64_t nodeId;
        uint64_t taskId;
    };

    struct TaskIDHash
    {
        size_t operator()(TaskID const& taskId) const noexcept
        {
            return taskId.nodeId ^ (taskId.taskId << 1);
        }
    };

    struct TaskEqualTo
    {
        bool operator()(const TaskID& taskId_l, const TaskID& taskId_r) const
        {
            return (taskId_l.nodeId == taskId_r.nodeId && taskId_l.taskId == taskId_r.taskId);
        }
    };

    struct TaskAccount
    {
        std::vector<uint64_t> Time;
        std::vector<uint64_t> nodeID;
    };


    //TODO：通过pair(NodeID,taskID)共同确定一个task
    std::unordered_map<TaskID, TaskAccount, TaskIDHash, TaskEqualTo> taskIDToInfo;

    std::ofstream* os_;

    static AccountTaskInfo* m_pInstance;

    AccountTaskInfo() { }

    AccountTaskInfo(AccountTaskInfo& instance) = default;
    AccountTaskInfo(AccountTaskInfo&& instance) = default;
    AccountTaskInfo& operator=(const AccountTaskInfo& instance) = default;

    void handleTaskSubmit(TaskID taskId, uint64_t Time)
    {
        taskIDToInfo[taskId].Time.push_back(Time);
        taskIDToInfo[taskId].nodeID.push_back(taskId.nodeId);
    }

    void handleTaskResourceSatisfy(TaskID taskId, uint64_t Time)
    {
        taskIDToInfo[taskId].Time.push_back(Time);
    }

    void handleTaskScheduled(TaskID taskId, uint64_t Time, uint64_t ScheduledNodeID)
    {
        taskIDToInfo[taskId].Time.push_back(Time);
        taskIDToInfo[taskId].nodeID.push_back(ScheduledNodeID);
    }

    void handleTaskStartExec(TaskID taskId, uint64_t Time)
    {
        taskIDToInfo[taskId].Time.push_back(Time);
    }

    void handleTaskEndExec(TaskID taskId, uint64_t Time)
    {
        taskIDToInfo[taskId].Time.push_back(Time);
        writeToFile(taskId);
        taskIDToInfo.erase(taskId);
    }

    void writeToFile(TaskID taskId)
    {
        TaskAccount account = taskIDToInfo[taskId];
        std::ofstream& ref_os_ = *os_;

        rapidjson::Document document;
        rapidjson::Value v_task;
        rapidjson::Value v_Time;
        rapidjson::Value v_nodeID;

        rapidjson::Document::AllocatorType& docAlloc = document.GetAllocator();

        document.SetObject();

        v_task.SetArray();
        v_task.PushBack(taskId.nodeId, docAlloc).PushBack(taskId.taskId, docAlloc);
        v_Time.SetArray();
        for (auto& iter : account.Time)
            v_Time.PushBack(iter, docAlloc);
        v_nodeID.SetArray();
        for (auto& iter : account.nodeID)
            v_nodeID.PushBack(iter, docAlloc);

        document.AddMember("ID", v_task, docAlloc);
        document.AddMember("Time", v_Time, docAlloc);
        document.AddMember("NodeID", v_nodeID, docAlloc);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> write(buffer);
        document.Accept(write);
        std::string json = buffer.GetString();
        ref_os_ << json;
        ref_os_ << std::endl;
    }


public:

    static AccountTaskInfo& getInstance()
    {
        if (m_pInstance == nullptr)
            m_pInstance = new AccountTaskInfo();
        return *m_pInstance;
    }

    ~AccountTaskInfo()
    {
        AccountTaskInfo::putInstance();
    }

    static void putInstance()
    {
        if (m_pInstance)
        {
            AccountTaskInfo* pDelete = m_pInstance;
            m_pInstance = nullptr;
            delete pDelete;
        }
    }

    void SetOutputStream(std::ofstream* os)
    {
        os_ = os;
    }

    //TODO: 处理节点变更
    void Account(AccountTaskInfo_ExecStep es, uint64_t taskID, uint64_t nodeID, uint64_t time, uint64_t scheduledNodeID = 0)
    {
        switch (es)
        {
        case TaskSubmit:
        handleTaskSubmit({ nodeID, taskID }, time);
        break;
        case TaskResourceSatisfy:
        handleTaskResourceSatisfy({ nodeID, taskID }, time);
        break;
        case TaskScheduled:
        handleTaskScheduled({ nodeID, taskID }, time, scheduledNodeID);
        break;
        case TaskStartExec:
        handleTaskStartExec({ nodeID, taskID }, time);
        break;
        case TaskEndExec:
        handleTaskEndExec({ nodeID, taskID }, time);
        break;
        }
    }
};