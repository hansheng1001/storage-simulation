#pragma once

#include "CPUAndMemoryConfigStoreBuffer.h"
#include "DiskConfigStoreBuffer.h"
#include "NetDriverConfigStoreBuffer.h"
#include <unordered_map>
#include <string>

class ServerConfigStoreBuffer
{
public:
    static std::unordered_map <std::string, CPUAndMemoryConfigStoreBuffer> ServerCPUAndMemoryConfigMap_;
    static std::unordered_map <std::string, DiskConfigStoreBuffer> ServerDiskConfigMap_;
    static std::unordered_map <std::string, NetDriverConfigStoreBuffer> ServerNetDriverConfigMap_;
};