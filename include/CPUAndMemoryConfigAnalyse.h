#pragma once

#include "rapidjson/document.h"
#include "CPUAndMemoryConfigStoreBuffer.h"
#include "ServerConfigMapStoreBuffer.h"
#include <string>

class CPUAndMemoryConfigAnalyseAndStore
{
public:
    
    static void Analyse(rapidjson::GenericObject<false, rapidjson::Value>& Object, std::string ServerClassName)
    {
        auto CPUAndMemoryKeyValue = Object.FindMember("CPUAndMemory");
        auto ObjCPUs = CPUAndMemoryKeyValue->value.FindMember("CPU")->value.GetObject();
        auto ObjMemorys = CPUAndMemoryKeyValue->value.FindMember("Memory")->value.GetObject();

        CPUAndMemoryConfigStoreBuffer Buffer;

        CPUConfigStore CPUConfigTemp;
        MemoryConfigStore MemoryConfigTemp;

        for (auto& cpu : ObjCPUs)
        {
            CPUConfigTemp.Cycle_ = cpu.value["Cycle"].GetDouble();
            CPUConfigTemp.CPI_ = cpu.value["CPI"].GetDouble();
            CPUConfigTemp.CoreNum_ = cpu.value["CoreNum"].GetUint();
            CPUConfigTemp.Node_ = cpu.value["Node"].GetUint();
            Buffer.Sockets_.push_back(CPUConfigTemp);
        }

        for (auto& mem : ObjMemorys)
        {
            MemoryConfigTemp.Size_ = mem.value["Size"].GetUint64();
            MemoryConfigTemp.LatencyFactor_ = mem.value["LantencyFactor"].GetDouble();
            Buffer.MemoryNodes_.push_back(MemoryConfigTemp);
        }

        Buffer.NUMALatencyFactor_ = CPUAndMemoryKeyValue->value["NUMALatencyFactor"].GetDouble();
        ServerConfigStoreBuffer::ServerCPUAndMemoryConfigMap_[ServerClassName] = Buffer;
    }
};
