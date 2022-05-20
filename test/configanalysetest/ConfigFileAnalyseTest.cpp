#include "InitConfigFileAnalyse.h"
#include "CPUAndMemoryConfigAnalyse.h"
#include "DiskConfigAnalyse.h"
#include <gtest/gtest.h>

TEST(ConfigFileAnalyse, func_all)
{
	bool OpenFileFlag = false;
	ConfigFileAnalyse CFA;

	auto& test1 = ServerConfigStoreBuffer::ServerCPUAndMemoryConfigMap_;
	auto& test2 = ServerConfigStoreBuffer::ServerDiskConfigMap_;
	auto& test3 = ServerConfigStoreBuffer::ServerNetDriverConfigMap_;

	OpenFileFlag = CFA.Init("ConfigFileTest.json");
	ASSERT_TRUE(OpenFileFlag);
	if (!OpenFileFlag)
		return;

	CFA.Analyse();

	/*DataServer配置解析test*/
	{
		//CPUAndMemory Config Test
		{
			ASSERT_DOUBLE_EQ(test1["DataServer"].Sockets_[0].Cycle_, 1.0);
			ASSERT_DOUBLE_EQ(test1["DataServer"].Sockets_[0].CPI_, 2.0);
			ASSERT_EQ(test1["DataServer"].Sockets_[0].CoreNum_, 32);
			ASSERT_EQ(test1["DataServer"].Sockets_[0].Node_, 0);

			ASSERT_DOUBLE_EQ(test1["DataServer"].Sockets_[1].Cycle_, 10.0);
			ASSERT_DOUBLE_EQ(test1["DataServer"].Sockets_[1].CPI_, 20.0);
			ASSERT_EQ(test1["DataServer"].Sockets_[1].CoreNum_, 32);
			ASSERT_EQ(test1["DataServer"].Sockets_[1].Node_, 1);

			ASSERT_DOUBLE_EQ(test1["DataServer"].Sockets_[2].Cycle_, 10.0);
			ASSERT_DOUBLE_EQ(test1["DataServer"].Sockets_[2].CPI_, 20.0);
			ASSERT_EQ(test1["DataServer"].Sockets_[2].CoreNum_, 32);
			ASSERT_EQ(test1["DataServer"].Sockets_[2].Node_, 0);

			ASSERT_EQ(test1["DataServer"].MemoryNodes_[0].Size_, 137438953472);
			ASSERT_DOUBLE_EQ(test1["DataServer"].MemoryNodes_[0].LatencyFactor_, 1.5);

			ASSERT_EQ(test1["DataServer"].MemoryNodes_[1].Size_, 137438953472);
			ASSERT_DOUBLE_EQ(test1["DataServer"].MemoryNodes_[1].LatencyFactor_, 1.5);

			ASSERT_DOUBLE_EQ(test1["DataServer"].NUMALatencyFactor_, 1.0);
		}

		//Disk Config Test
		{
			ASSERT_EQ(test2["DataServer"].Disks_[0].ChannelNum_, 1);
			ASSERT_EQ(test2["DataServer"].Disks_[0].MaxReadBandwidth_, 2);
			ASSERT_EQ(test2["DataServer"].Disks_[0].MaxWriteBandWidth_, 3);
			ASSERT_EQ(test2["DataServer"].Disks_[0].MaxReadBandWidthPerChannel_, 4);
			ASSERT_EQ(test2["DataServer"].Disks_[0].MaxWriteBandWidthPerChannel_, 5);
			ASSERT_EQ(test2["DataServer"].Disks_[0].ReadBaseLatency_, 6);
			ASSERT_EQ(test2["DataServer"].Disks_[0].WriteBaseLatency_, 7);

			ASSERT_EQ(test2["DataServer"].Disks_[1].ChannelNum_, 10);
			ASSERT_EQ(test2["DataServer"].Disks_[1].MaxReadBandwidth_, 20);
			ASSERT_EQ(test2["DataServer"].Disks_[1].MaxWriteBandWidth_, 30);
			ASSERT_EQ(test2["DataServer"].Disks_[1].MaxReadBandWidthPerChannel_, 40);
			ASSERT_EQ(test2["DataServer"].Disks_[1].MaxWriteBandWidthPerChannel_, 50);
			ASSERT_EQ(test2["DataServer"].Disks_[1].ReadBaseLatency_, 60);
			ASSERT_EQ(test2["DataServer"].Disks_[1].WriteBaseLatency_, 70);

			ASSERT_EQ(test2["DataServer"].Disks_[2].ChannelNum_, 100);
			ASSERT_EQ(test2["DataServer"].Disks_[2].MaxReadBandwidth_, 200);
			ASSERT_EQ(test2["DataServer"].Disks_[2].MaxWriteBandWidth_, 300);
			ASSERT_EQ(test2["DataServer"].Disks_[2].MaxReadBandWidthPerChannel_, 400);
			ASSERT_EQ(test2["DataServer"].Disks_[2].MaxWriteBandWidthPerChannel_, 500);
			ASSERT_EQ(test2["DataServer"].Disks_[2].ReadBaseLatency_, 600);
			ASSERT_EQ(test2["DataServer"].Disks_[2].WriteBaseLatency_, 700);
		}
		//NetDriver Config Test
		{
			ASSERT_FLOAT_EQ(test3["DataServer"].NetDriver_[0].SendDelay_, 43.0);
			ASSERT_FLOAT_EQ(test3["DataServer"].NetDriver_[0].InternetDelay_, 36.0);
			ASSERT_FLOAT_EQ(test3["DataServer"].NetDriver_[0].DownBandWidth_, 12500.0);
			ASSERT_FLOAT_EQ(test3["DataServer"].NetDriver_[0].UpBandWidth_, 100000.0);
		}
	}

	/*ComputeServer配置解析test*/
	{
		//CPUAndMemory Config Test
		{
			ASSERT_DOUBLE_EQ(test1["ComputeServer"].Sockets_[0].Cycle_, 1.0);
			ASSERT_DOUBLE_EQ(test1["ComputeServer"].Sockets_[0].CPI_, 2.0);
			ASSERT_EQ(test1["ComputeServer"].Sockets_[0].CoreNum_, 32);
			ASSERT_EQ(test1["ComputeServer"].Sockets_[0].Node_, 0);

			ASSERT_DOUBLE_EQ(test1["ComputeServer"].Sockets_[1].Cycle_, 10.0);
			ASSERT_DOUBLE_EQ(test1["ComputeServer"].Sockets_[1].CPI_, 20.0);
			ASSERT_EQ(test1["ComputeServer"].Sockets_[1].CoreNum_, 32);
			ASSERT_EQ(test1["ComputeServer"].Sockets_[1].Node_, 1);

			ASSERT_DOUBLE_EQ(test1["ComputeServer"].Sockets_[2].Cycle_, 10.0);
			ASSERT_DOUBLE_EQ(test1["ComputeServer"].Sockets_[2].CPI_, 20.0);
			ASSERT_EQ(test1["ComputeServer"].Sockets_[2].CoreNum_, 32);
			ASSERT_EQ(test1["ComputeServer"].Sockets_[2].Node_, 0);

			ASSERT_EQ(test1["ComputeServer"].MemoryNodes_[0].Size_, 137438953472);
			ASSERT_DOUBLE_EQ(test1["ComputeServer"].MemoryNodes_[0].LatencyFactor_, 1.5);

			ASSERT_EQ(test1["ComputeServer"].MemoryNodes_[1].Size_, 137438953472);
			ASSERT_DOUBLE_EQ(test1["ComputeServer"].MemoryNodes_[1].LatencyFactor_, 1.5);

			ASSERT_DOUBLE_EQ(test1["ComputeServer"].NUMALatencyFactor_, 1.0);
		}

		//Disk Config Test
		{
			ASSERT_EQ(test2["ComputeServer"].Disks_[0].ChannelNum_, 1);
			ASSERT_EQ(test2["ComputeServer"].Disks_[0].MaxReadBandwidth_, 2);
			ASSERT_EQ(test2["ComputeServer"].Disks_[0].MaxWriteBandWidth_, 3);
			ASSERT_EQ(test2["ComputeServer"].Disks_[0].MaxReadBandWidthPerChannel_, 4);
			ASSERT_EQ(test2["ComputeServer"].Disks_[0].MaxWriteBandWidthPerChannel_, 5);
			ASSERT_EQ(test2["ComputeServer"].Disks_[0].ReadBaseLatency_, 6);
			ASSERT_EQ(test2["ComputeServer"].Disks_[0].WriteBaseLatency_, 7);

			ASSERT_EQ(test2["ComputeServer"].Disks_[1].ChannelNum_, 10);
			ASSERT_EQ(test2["ComputeServer"].Disks_[1].MaxReadBandwidth_, 20);
			ASSERT_EQ(test2["ComputeServer"].Disks_[1].MaxWriteBandWidth_, 30);
			ASSERT_EQ(test2["ComputeServer"].Disks_[1].MaxReadBandWidthPerChannel_, 40);
			ASSERT_EQ(test2["ComputeServer"].Disks_[1].MaxWriteBandWidthPerChannel_, 50);
			ASSERT_EQ(test2["ComputeServer"].Disks_[1].ReadBaseLatency_, 60);
			ASSERT_EQ(test2["ComputeServer"].Disks_[1].WriteBaseLatency_, 70);

			ASSERT_EQ(test2["ComputeServer"].Disks_[2].ChannelNum_, 100);
			ASSERT_EQ(test2["ComputeServer"].Disks_[2].MaxReadBandwidth_, 200);
			ASSERT_EQ(test2["ComputeServer"].Disks_[2].MaxWriteBandWidth_, 300);
			ASSERT_EQ(test2["ComputeServer"].Disks_[2].MaxReadBandWidthPerChannel_, 400);
			ASSERT_EQ(test2["ComputeServer"].Disks_[2].MaxWriteBandWidthPerChannel_, 500);
			ASSERT_EQ(test2["ComputeServer"].Disks_[2].ReadBaseLatency_, 600);
			ASSERT_EQ(test2["ComputeServer"].Disks_[2].WriteBaseLatency_, 700);
		}


		//NetDriver Config Test
		{
			ASSERT_FLOAT_EQ(test3["ComputeServer"].NetDriver_[0].SendDelay_, 43.0);
			ASSERT_FLOAT_EQ(test3["ComputeServer"].NetDriver_[0].InternetDelay_, 36.0);
			ASSERT_FLOAT_EQ(test3["ComputeServer"].NetDriver_[0].DownBandWidth_, 12500.0);
			ASSERT_FLOAT_EQ(test3["ComputeServer"].NetDriver_[0].UpBandWidth_, 100000.0);
		}
	}
}

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}