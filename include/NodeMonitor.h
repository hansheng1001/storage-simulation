#pragma once
#include <memory>
#include <vector>
#include "Analyzer.h"
#include "Base.h"
#include "EventLoop.h"
#include "NodeManager.h"

template<typename Archive, typename OutputStream>
class NodeMonitor
{
    OutputStream os_;
    std::vector<Analyzer<Archive>> analyzers_;
    Time samplingPeriod_;
    bool stopSampling_;

    void sampleOneNode(const Node& node)
    {
        Archive archive(os_);
        archive.init();
        for (auto& analyzer : analyzers_)
        {
            analyzer(node, archive);
        }
        archive.WriteToFile();
    }

    void sample(){
        NodeManager::getIntance().forEachNode([this](const Node& node){
            sampleOneNode(node);
        });

        if(!stopSampling_)
            EventLoop::getInstance().callAfter(samplingPeriod_, [this](){
                sample();
            });
    }

public:
    NodeMonitor(OutputStream&& os, Time samplingPeriod):
        os_(std::move(os)),
        samplingPeriod_(samplingPeriod),
        stopSampling_(false)
    {}

    void addAnalyzer(const Analyzer<Archive>& analyzer){
        analyzers_.push_back(analyzer);
    }

    void startMonitoring(){
        EventLoop::getInstance().callAfter(samplingPeriod_, [this](){
            sample();
        });
    }

    void stopMonitoring(){
        stopSampling_ = true;
    }
};