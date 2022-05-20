#include "ServerConfigMapStoreBuffer.h"


std::unordered_map <std::string, CPUAndMemoryConfigStoreBuffer> ServerConfigStoreBuffer::ServerCPUAndMemoryConfigMap_;
std::unordered_map <std::string, DiskConfigStoreBuffer> ServerConfigStoreBuffer::ServerDiskConfigMap_;
std::unordered_map <std::string, NetDriverConfigStoreBuffer> ServerConfigStoreBuffer::ServerNetDriverConfigMap_;