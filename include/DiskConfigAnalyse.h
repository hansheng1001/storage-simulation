#pragma once

#include "rapidjson/document.h"
#include "ServerConfigMapStoreBuffer.h"
#include "DiskConfigStoreBuffer.h"

class DiskConfigAnalyseAndStore
{
public:
	
	static void Analyse(rapidjson::GenericObject<false, rapidjson::Value>& Object, std::string ServerClassName)
	{
		auto DiskKeyValue = Object.FindMember("Disk");

		auto ObjDisk = DiskKeyValue->value.GetObject();

		DiskConfigStoreBuffer Buffer;

		DiskConfigStore DiskConfigTemp;

		for (auto& it : ObjDisk)
		{
			DiskConfigTemp.ChannelNum_ = it.value["ChannelNum"].GetUint();
			DiskConfigTemp.MaxReadBandwidth_ = it.value["MaxReadBandwidth"].GetUint64();
			DiskConfigTemp.MaxWriteBandWidth_ = it.value["MaxWriteBandWidth"].GetUint64();
			DiskConfigTemp.MaxReadBandWidthPerChannel_ = it.value["MaxReadBandWidthPerChannel"].GetUint64();
			DiskConfigTemp.MaxWriteBandWidthPerChannel_ = it.value["MaxWriteBandWidthPerChannel"].GetUint64();
			DiskConfigTemp.ReadBaseLatency_ = it.value["ReadBaseLatency"].GetUint64();
			DiskConfigTemp.WriteBaseLatency_ = it.value["WriteBaseLatency"].GetUint64();
			Buffer.Disks_.push_back(DiskConfigTemp);
		}

		ServerConfigStoreBuffer::ServerDiskConfigMap_[ServerClassName] = Buffer;
	}
};