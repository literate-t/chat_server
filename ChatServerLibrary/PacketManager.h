#pragma once
#include "stdafx.h"

using namespace Common;
namespace ChatServerLibrary
{
	class UserManager;
	class LobbyManager;
	class PacketManager
	{
	public:
		PacketManager() = default;
		~PacketManager() = default;

		void Init(UserManager* user_manager, LobbyManager* lobby_manager, ServerLibrary::ILog* log);
		void ProcessPacket(int session_index, char* buf, short copy_size);
		void Login(int session_index, char* buf, short copy_size);
		void LobbyEnter(int session_index, char* buf, short copy_size);
		void LobbyLeave(int session_index, char* buf, short copy_size);
		void RoomEnter(int session_index, char* buf, short copy_size);
		void RoomLeave(int session_index, char* buf, short copy_size);
		void RoomChat(int session_index, char* buf, short copy_size);
		bool ProcessLogoff(int session_index);

		function<void(int, void*, short)> SendPacketFunc;

	private:
		using PacketFunc = void(PacketManager::*)(int, char*, short);
		std::unordered_map<short, PacketFunc> PacketFuncDictionary;

		UserManager* user_mgr_		= nullptr;
		LobbyManager* lobby_mgr_		= nullptr;
		ServerLibrary::ILog* log_	= nullptr;
	};
}