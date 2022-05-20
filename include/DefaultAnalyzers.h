#pragma once
#include "Base.h"
#include "Node.h"
#include "EventLoop.h"
#include "cereal/cereal.hpp"

#define RAW_NAME(T) CEREAL_NVP(T)

void getIDAndTime(const Node& node, NodeID& id, Time& time)
{
    id = node.getNodeID();
    time = EventLoop::getInstance().GetCurrentTime();
}

template<typename ResType>
struct AnalysisOutput
{
    NodeID nodeID;
    Time samplingTime;
    ResType analysisResult;

    AnalysisOutput(const Node& node) :nodeID(0), samplingTime(0)
    {
        getIDAndTime(node, nodeID, samplingTime);
    }

    template<typename Archive>
    void serialize(Archive& archive)    
{
        archive(std::string{ "nodeID&Time" }, 2, nodeID, samplingTime);
        analysisResult.serialize(archive);
    }
};

template<typename Archive>
class CPUUtilizationAnalyzer
{
public:

    struct AnalysisResult
    {
        double CPUUtilization;
        unsigned totalCoreNum;
        unsigned usedCoreNum;
        AnalysisResult() :CPUUtilization(0.0), totalCoreNum(0), usedCoreNum(0) {}
        void serialize(Archive& archive)
        {
            archive(
                std::string{ "CPU" }, 3,
                CPUUtilization,
                totalCoreNum,
                usedCoreNum
            );
        }
    };

public:
    void operator()(const Node& node, Archive& archive)
    {
        AnalysisOutput<AnalysisResult> output(node);

        auto& state = node.getComputationArchRuntimeState();
        auto& arch = node.getComputationArch();

        unsigned idleCoreNum = 0;

        for (auto& cpuState : state.cpuStates_)
            idleCoreNum += cpuState.getIdleCoreNum();
        for (auto& cpu : arch.sockets_)
            output.analysisResult.totalCoreNum += cpu.coreNum_;

        output.analysisResult.usedCoreNum = output.analysisResult.totalCoreNum - idleCoreNum;
        output.analysisResult.CPUUtilization = (double)output.analysisResult.usedCoreNum / output.analysisResult.totalCoreNum;

        output.serialize(archive);
    }
};

template<typename Archive>
class MemoryUtilizationAnalyzer
{
    struct AnalysisResult
    {
        double memoryUtilization;
        unsigned long totalMemory;
        unsigned long usedMemory;

        AnalysisResult() :memoryUtilization(0.0), totalMemory(0), usedMemory(0) {}

        void serialize(Archive& archive)
        {
            archive(
                std::string{ "Memory" }, 3,
                memoryUtilization,
                totalMemory,
                usedMemory
            );
        }
    };

public:
    void operator()(const Node& node, Archive& archive)
    {
        AnalysisOutput<AnalysisResult> output(node);
        auto& state = node.getComputationArchRuntimeState();
        auto& arch = node.getComputationArch();

        unsigned long memoryLeft = 0;

        for (auto left : state.nodesMemoryLeft_)
            memoryLeft += left;
        for (auto& mem : arch.memoryNodes_)
            output.analysisResult.totalMemory += mem.size_;

        output.analysisResult.usedMemory = output.analysisResult.totalMemory - memoryLeft;
        output.analysisResult.memoryUtilization = (double)output.analysisResult.usedMemory / output.analysisResult.totalMemory;

        output.serialize(archive);
    }
};

struct PerDiskDetail
{
    unsigned usedChannel;
    unsigned totalChannel;
    unsigned long usedReadBandwidth;
    unsigned long totalReadBandwidth;
    unsigned long usedWriteBandwidth;
    unsigned long totalWriteBandwidth;

    PerDiskDetail() :
        usedChannel(0), totalChannel(0), usedReadBandwidth(0),
        totalReadBandwidth(0), usedWriteBandwidth(0), totalWriteBandwidth(0)
    {}

    // void serialize(Archive& archive){
    //     archive(
    //         usedChannel, totalChannel,
    //         usedReadBandwidth, totalReadBandwidth,
    //         usedWriteBandwidth, totalWriteBandwidth
    //     );
    // }
};

template<typename Archive>
class DiskAnalyzer
{
    struct Summarization
    {
        unsigned long usedReadBandwidth;
        unsigned long totalReadBandwidth;
        unsigned long usedWriteBandwidth;
        unsigned long totalWriteBandwidth;
        vector<PerDiskDetail> details;

        Summarization() :usedReadBandwidth(0), totalReadBandwidth(0), usedWriteBandwidth(0), totalWriteBandwidth(0) {};

        void serialize(Archive& archive)
        {
            archive(
                std::string{ "Disk" }, 5,
                usedReadBandwidth, totalReadBandwidth,
                usedWriteBandwidth, totalWriteBandwidth,
                &details
            );
        }
    };

public:
    void operator()(const Node& node, Archive& archive)
    {
        AnalysisOutput<Summarization> output(node);
        unsigned diskNum = node.getDiskNum();

        for (unsigned i = 0; i < diskNum; ++i)
        {
            PerDiskDetail detail;
            const Disk& disk = node.getDiskInfo(i);
            auto& diskState = node.getDiskRunTimeState(i);

            detail.totalChannel = disk.channelNum_;
            detail.totalReadBandwidth = disk.maxReadBandwidth_;
            detail.totalWriteBandwidth = disk.maxWriteBandWidth_;

            detail.usedChannel = disk.channelNum_ - diskState.channelLeft_;
            detail.usedReadBandwidth = disk.maxReadBandwidth_ - diskState.readBandWidthLeft_;
            detail.usedWriteBandwidth = disk.maxWriteBandWidth_ - diskState.writeBandWidthLeft_;

            output.analysisResult.details.push_back(detail);

            output.analysisResult.usedReadBandwidth += detail.usedReadBandwidth;
            output.analysisResult.usedWriteBandwidth += detail.usedWriteBandwidth;
            output.analysisResult.totalReadBandwidth += disk.maxReadBandwidth_;
            output.analysisResult.totalWriteBandwidth += disk.maxWriteBandWidth_;
        }

        output.serialize(archive);
    }
};

template<typename Archive>
class NetUtilizationAnalyzer
{
    struct AnalysisResult
    {
        double totalusedsendcircleSize;//总接收带宽
        double totalusedrecivecircleSize;//总发送带宽
        double netSendUtilization;
        double netReciveUtilization;

        AnalysisResult() :
            totalusedsendcircleSize(0.0), totalusedrecivecircleSize(0.0),
            netSendUtilization(0.0), netReciveUtilization(0.0)
        {}

        void serialize(Archive& archive)
        {
            archive(
                std::string{ "Net" }, 4,
                totalusedsendcircleSize, totalusedrecivecircleSize,
                netSendUtilization, netReciveUtilization
                );
        }
    };

public:
    void operator()(const Node& node, Archive& archive)
    {
        AnalysisOutput<AnalysisResult> output(node);
        NetDirverCircle netInfo = node.getNetDriverInfo();

        float unusedsendcircleSize;
        float unusedrecivecircleSize;
        float totalsendcircleSize;
        float totalrecivecircleSize;

        unusedsendcircleSize = netInfo.getUnusedSendCircleSize();
        unusedrecivecircleSize = netInfo.getUnusedReciveCircleSize();
        totalsendcircleSize = netInfo.getTotalSendCircleSize();
        totalrecivecircleSize = netInfo.getTotalReciveCircleSize();

        output.analysisResult.totalusedsendcircleSize = totalsendcircleSize;
        output.analysisResult.totalusedrecivecircleSize = totalrecivecircleSize;
        output.analysisResult.netSendUtilization = (totalsendcircleSize - unusedsendcircleSize) / totalsendcircleSize;
        output.analysisResult.netReciveUtilization = (totalrecivecircleSize - unusedrecivecircleSize) / totalrecivecircleSize;

        output.serialize(archive);
    }
};