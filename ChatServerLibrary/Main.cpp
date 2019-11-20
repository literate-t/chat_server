#include "stdafx.h"
#include "../ServerLibrary/Logger.h"
//#include "Main.h"


namespace ChatServerLibrary
{
	int Main::Init()
	{
		Config = make_unique <ServerLibrary::ServerConfig>();
		LoadConfig();

		Log = make_unique<ServerLibrary::Logger>();

		Server = make_unique<ServerLibrary::IocpServer>();
		Server->Init(Config.get(), Log.get());
		auto result = Server->Start();
		if (result == false)
		{
			Log->Write(ServerLibrary::LogType::L_ERROR, "%s | Starting server is failed", __FUNCTION__);
			return -1;
		}

		auto SendPacketFunc = [&](int connectionIndex, void* packet, short packetSize)
		{
			Server->SendPacket(connectionIndex, packet, packetSize);
		};

		UserMgr = make_unique<UserManager>();
		UserMgr->Init(Config->MaxSessionCount);
		UserMgr->SendPacketFunc = SendPacketFunc;

		LobbyMgr = make_unique<LobbyManager>();
		LobbyManagerConfig config = { Config->MaxLobbyCount, Config->MaxLobbyUserCount,
									  Config->MaxRoomCount, Config->MaxRoomUserCount };
		LobbyMgr->SendPacketFunc = SendPacketFunc;
		LobbyMgr->Init(&config, Server.get(), Log.get());

		PacketMgr = make_unique<PacketManager>();
		PacketMgr->Init(UserMgr.get(), LobbyMgr.get(), Log.get());
		PacketMgr->SendPacketFunc = SendPacketFunc;

		return 0;
	}

	void Main::Run()
	{
		IsRunning = true;
		auto buf = new char[Config->MaxPacketSize];
		memset(buf, 0, Config->MaxPacketSize);
		int waitMilliseconds = 1;

		while (IsRunning)
		{
			char type = 0;
			int sessionIndex = 0;
			short copySize = 0;

			if (!Server->ProcessIocpMessage(type, sessionIndex, &buf, copySize, waitMilliseconds))
			{
				continue;
			}

			auto msgType = static_cast<ServerLibrary::MessageType>(type);
			switch (msgType)
			{
				case ServerLibrary::MessageType::CONNECTION:
				{
					Log->Write(ServerLibrary::LogType::L_INFO, "On connect index:%d", sessionIndex);
					break;
				}

				case ServerLibrary::MessageType::CLOSE:
				{
					Log->Write(ServerLibrary::LogType::L_INFO, "On close index:%d close process is needed", sessionIndex);
					break;
				}

				case ServerLibrary::MessageType::ONRECV:
				{
					PacketMgr->ProcessPacket(sessionIndex, buf, copySize);
					break;
				}
			}
		}
		delete[] buf;
	}

	void Main::Stop()
	{
		IsRunning = false;
		Server->End();
	}

	void Main::LoadConfig()
	{
		wchar_t path[MAX_PATH] = { 0 };
		GetCurrentDirectory(MAX_PATH, path);

		wchar_t configPath[MAX_PATH] = { 0 };
		_snwprintf_s(configPath, MAX_PATH, _TRUNCATE, L"%s\\ServerConfig.ini", path);

		Config->Port = (unsigned short)GetPrivateProfileInt(L"ServerConfig", L"Port", 0,  configPath);
		Config->BackLogCount = GetPrivateProfileInt(L"ServerConfig", L"BackLogCount", 0, configPath);
		Config->WorkerThreadCount = GetPrivateProfileInt(L"ServerConfig", L"WorkerThreadCount", 0, configPath);
		Config->MaxRecvOverlappedBufferSize = GetPrivateProfileInt(L"ServerConfig", L"MaxRecvOverlappedBufferSize", 0, configPath);
		Config->MaxSendOverlappedBufferSize =	GetPrivateProfileInt(L"ServerConfig", L"MaxSendOverlappedBufferSize", 0, configPath);
		Config->SessionMaxRecvBufferSize =	GetPrivateProfileInt(L"ServerConfig", L"SessionMaxRecvBufferSize", 0, configPath);
		Config->SessionMaxSendBufferSize =	GetPrivateProfileInt(L"ServerConfig", L"SessionMaxSendBufferSize", 0, configPath);
		Config->MaxPacketSize =	GetPrivateProfileInt(L"ServerConfig", L"MaxPacketSize", 0, configPath);
		Config->MaxSessionCount =	GetPrivateProfileInt(L"ServerConfig", L"MaxSessionCount", 0, configPath);
		Config->MaxMessagePoolCount =	GetPrivateProfileInt(L"ServerConfig", L"MaxMessagePoolCount", 0, configPath);
		Config->ExtraMessagePoolCount =	GetPrivateProfileInt(L"ServerConfig", L"ExtraMessagePoolCount", 0, configPath);
		Config->PerformancePacketMillisecondsTime = GetPrivateProfileInt(L"ServerConfig", L"PerformancePacketMillisecondsTime", 0, configPath);

		Config->MaxLobbyCount = GetPrivateProfileInt(L"ServerConfig", L"MaxLobbyCount", 0, configPath);
		Config->MaxLobbyUserCount = GetPrivateProfileInt(L"ServerConfig", L"MaxLobbyUserCount", 0, configPath);
		Config->MaxRoomCount = GetPrivateProfileInt(L"ServerConfig", L"MaxRoomCount", 0, configPath);
		Config->MaxRoomUserCount = GetPrivateProfileInt(L"ServerConfig", L"MaxRoomUserCount", 0, configPath);
	}
}
