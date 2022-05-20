#pragma once
#include <string>
#include <fstream>
#include <unordered_map>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <stdarg.h>

#include "DefaultAnalyzers.h"
#include "NodeMonitor.h"

static std::unordered_map<std::string, std::string> TypeToID
{
    {"nodeID&Time",     "0"},
    {"CPU",             "1"},
    {"Memory",          "2"},
    {"Disk",            "3"},
    {"Net",             "4"}
};

static void IDTimeJsonFunc(rapidjson::Document& document, int count, va_list& args)
{
    NodeID nodeID = va_arg(args, NodeID);
    Time samplingTime = va_arg(args, Time);

    rapidjson::Document::AllocatorType& docAlloc = document.GetAllocator();
    rapidjson::Value arrNodeIDTime;

    if (document.HasMember("0"))
    {
        return;
    }
    else
    {
        arrNodeIDTime.SetArray();
        arrNodeIDTime.PushBack(nodeID, docAlloc).PushBack(samplingTime, docAlloc);
        document.AddMember("0", arrNodeIDTime, docAlloc);
    }
}

static void CPUJsonFunc(rapidjson::Document& document, int count, va_list& args)
{
    double CPUUtiliaztion = va_arg(args, double);
    unsigned totalCoreNum = va_arg(args, unsigned);
    unsigned usedCoreNum = va_arg(args, unsigned);

    rapidjson::Document::AllocatorType& docAlloc = document.GetAllocator();
    rapidjson::Value arrCPU;

    arrCPU.SetArray();

    arrCPU.PushBack(CPUUtiliaztion, docAlloc).PushBack(totalCoreNum, docAlloc).PushBack(usedCoreNum, docAlloc);

    document.AddMember("1", arrCPU, docAlloc);
}

static void MemJsonFunc(rapidjson::Document& document, int count, va_list& args)
{
    double memoryUtilization = va_arg(args, double);
    unsigned long totalMemory = va_arg(args, unsigned long);
    unsigned long usedMemory = va_arg(args, unsigned long);

    rapidjson::Document::AllocatorType& docAlloc = document.GetAllocator();
    rapidjson::Value arrMem;

    arrMem.SetArray();

    arrMem.PushBack(memoryUtilization, docAlloc).PushBack(totalMemory, docAlloc).PushBack(usedMemory, docAlloc);

    document.AddMember("2", arrMem, docAlloc);
}

static void DiskJsonFunc(rapidjson::Document& document, int count, va_list& args)
{
    unsigned long usedReadBandwidth = va_arg(args, unsigned long);
    unsigned long totalReadBandwidth = va_arg(args, unsigned long);
    unsigned long usedWriteBandwidth = va_arg(args, unsigned long);
    unsigned long totalWriteBandwidth = va_arg(args, unsigned long);
    std::vector<PerDiskDetail>* details = va_arg(args, std::vector<PerDiskDetail>*);

    rapidjson::Document::AllocatorType& docAlloc = document.GetAllocator();
    rapidjson::Value arrDisk;
    rapidjson::Value arrDiskDetail;

    arrDisk.SetArray();
    arrDisk.PushBack(usedReadBandwidth, docAlloc).PushBack(totalReadBandwidth, docAlloc)
        .PushBack(usedWriteBandwidth, docAlloc).PushBack(totalWriteBandwidth, docAlloc);

    for (auto& iter : (*details))
    {
        arrDiskDetail.SetArray();
        arrDiskDetail.PushBack(iter.usedChannel, docAlloc).PushBack(iter.totalChannel, docAlloc)
            .PushBack(iter.usedReadBandwidth, docAlloc).PushBack(iter.totalReadBandwidth, docAlloc)
            .PushBack(iter.usedWriteBandwidth, docAlloc).PushBack(iter.totalWriteBandwidth, docAlloc);
        arrDisk.PushBack(arrDiskDetail, docAlloc);
    }

    document.AddMember("3", arrDisk, docAlloc);
}

static void NetJsonFunc(rapidjson::Document& document, int count, va_list& args)
{
    double usedsendcircleSize = va_arg(args,double);
    double usedrecivecircleSize = va_arg(args,double);
    double netSendUtilization = va_arg(args,double);
    double netReciveUtilization = va_arg(args,double);

    rapidjson::Document::AllocatorType& docAlloc = document.GetAllocator();
    rapidjson::Value arrNet;

    arrNet.SetArray();
    arrNet.PushBack(usedsendcircleSize, docAlloc).PushBack(usedrecivecircleSize, docAlloc).
        PushBack(netSendUtilization, docAlloc).PushBack(netReciveUtilization, docAlloc);

    document.AddMember("4", arrNet, docAlloc);
}


static std::unordered_map <std::string, std::function<void(rapidjson::Document&, int, va_list&)>> TypeToFunc
{
    {"nodeID&Time",         IDTimeJsonFunc},
    {"CPU",                 CPUJsonFunc},
    {"Memory",              MemJsonFunc},
    {"Disk",                DiskJsonFunc},
    {"Net",                 NetJsonFunc}
};

template<typename ouputStream>
class ArchiveNodeInfo
{
private:
    using JsonFunc = std::function<void(rapidjson::Document&, int, va_list&)>;
    ouputStream& os_;
    rapidjson::Document document;
    std::unordered_map<std::string, JsonFunc> TypeToJsonFunc;

public:
    ArchiveNodeInfo(ouputStream& os) :os_(os)
    {

    }

    void operator()(std::string InfoType, int count, ...)
    {
        va_list args;
        va_start(args, count);
        TypeToJsonFunc[InfoType](document, count, args);

        va_end(args);

    }

    void init()
    {
        document.SetObject();
        for (auto& iter : TypeToFunc)
        {
            TypeToJsonFunc[iter.first] = iter.second;
        }
    }

    void WriteToFile()
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> write(buffer);
        document.Accept(write);
        std::string json = buffer.GetString();
        os_ << json;
        os_ << std::endl;

        document.SetObject();
    }

};







