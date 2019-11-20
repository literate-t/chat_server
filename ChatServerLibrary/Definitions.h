#pragma once
#include "../ServerLibrary/Definitions.h"

namespace ChatServerLibrary
{
	const int kPacketHeaderLength = 4;
	const int kPacketSizeLength = 2;
	const int kPacketTypeLength = 2;

	struct ChatConfig : ServerLibrary::ServerConfig
	{
		int PostMessageThreadsCount = 1;
		int StartRoomNummber = 0;
		int MaxRoomCount = 0;
		int MaxRoomUserCount = 0;

		ServerLibrary::ServerConfig GetServerConfig()
		{
			ServerLibrary::ServerConfig Config;

			Config.Port = Port;
			Config.WorkerThreadCount = WorkerThreadCount;
			Config.MaxRecvOverlappedBufferSize = MaxRecvOverlappedBufferSize;
			Config.MaxSendOverlappedBufferSize = MaxSendOverlappedBufferSize;
			Config.ConnectionMaxRecvBufferSize = ConnectionMaxRecvBufferSize;
			Config.ConnectionMaxSendBufferSize = ConnectionMaxSendBufferSize;
			Config.MaxPacketSize = MaxPacketSize;
			Config.MaxConnectionCount = MaxConnectionCount;
			Config.MaxMessagePoolCount = MaxMessagePoolCount;
			Config.ExtraMessagePoolCount = ExtraMessagePoolCount;
			Config.PerformancePacketMillisecondsTime = PerformancePacketMillisecondsTime;

			return Config;
		}
	};
}

