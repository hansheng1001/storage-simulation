#pragma once

#include <vector>

struct NetDriverConfigStore
{
    float SendDelay_;
    float InternetDelay_;
    float DownBandWidth_;
    float UpBandWidth_;
};

struct NetDriverConfigStoreBuffer
{
    std::vector<NetDriverConfigStore> NetDriver_;
};