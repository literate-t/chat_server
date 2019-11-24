#pragma once
#include "../ServerLibrary/Definitions.h"

namespace ChatServerLibrary
{
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
			Config.SessionMaxRecvBufferSize = SessionMaxRecvBufferSize;
			Config.SessionMaxSendBufferSize = SessionMaxSendBufferSize;
			Config.MaxPacketSize = MaxPacketSize;
			Config.MaxSessionCount = MaxSessionCount;
			Config.MaxMessagePoolCount = MaxMessagePoolCount;
			Config.ExtraMessagePoolCount = ExtraMessagePoolCount;
			Config.PerformancePacketMillisecondsTime = PerformancePacketMillisecondsTime;

			return Config;
		}
	};
}