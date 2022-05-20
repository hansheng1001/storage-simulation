#pragma once

#include "rapidjson/document.h"
#include "NetDriverConfigStoreBuffer.h"
#include "ServerConfigMapStoreBuffer.h"
#include <string>

class NetDriverConfigAnalyseAndStore
{
public:
    static void Analyse(rapidjson::GenericObject<false, rapidjson::Value>& Object, std::string ServerClassName)
    {
        auto NetDriverKeyValue = Object.FindMember("NetDriver");

        auto ObjNetDriver = NetDriverKeyValue->value.GetObject();

        NetDriverConfigStoreBuffer Buffer;

        NetDriverConfigStore NetDriverConfigTemp;

        for (auto& it : ObjNetDriver)
        {
            NetDriverConfigTemp.SendDelay_ = it.value["SendDelay"].GetFloat();
            NetDriverConfigTemp.InternetDelay_ = it.value["InternetDelay"].GetFloat();
            NetDriverConfigTemp.DownBandWidth_ = it.value["DownBandWidth"].GetFloat();
            NetDriverConfigTemp.UpBandWidth_ = it.value["UpBandWidth"].GetFloat();
            Buffer.NetDriver_.push_back(NetDriverConfigTemp);
        }
        ServerConfigStoreBuffer::ServerNetDriverConfigMap_[ServerClassName] = Buffer;
    }
    
};