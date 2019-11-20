#pragma once
//#include <unordered_map>
//#include <functional>
//#include "../ServerLibrary/ILog.h"
#include "stdafx.h"

namespace ChatServerLibrary
{
	class UserManager;
	class LobbyManager;
	class PacketManager
	{
	public:
		PacketManager() = default;
		~PacketManager() = default;

		void Init(UserManager* userManager, LobbyManager* lobbyManager, ServerLibrary::ILog* log);
		void ProcessPacket(int connectionIndex, char* buf, short copySize);
		void ProcessLogin(int connectionIndex, char* buf, short copySize);
		void ProcessLogoff(int connectionIndex, char* buf, short copySize);


		function<void(int, void*, short)> SendPacketFunc;

	private:
		using PacketFunc = void(PacketManager::*)(int, char*, short);
		std::unordered_map<short, PacketFunc> PacketFuncDictionary;

		UserManager* UserMgr = nullptr;
		LobbyManager* LobbyMgr = nullptr;
		ServerLibrary::ILog* Log = nullptr;
	};
}