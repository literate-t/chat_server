#pragma once
#include "stdafx.h"

using namespace Common;
namespace chat_server_library
{
	class UserManager;
	class LobbyManager;
	class PacketManager
	{
	public:
		PacketManager() = default;
		~PacketManager() = default;

		void Init(UserManager* user_manager, LobbyManager* lobby_manager, server_library::ILog* log);
		void ProcessPacket(const int session_index, char* buf, const short copy_size);
		void Login(const int session_index, char* buf, const short copy_size);
		void LobbyEnter(const int session_index, char* buf, const short copy_size);
		void LobbyLeave(const int session_index, char* buf, const short copy_size);
		void RoomEnter(const int session_index, char* buf, const short copy_size);
		void RoomLeave(const int session_index, char* buf, const short copy_size);
		void RoomChat(const int session_index, char* buf, const short copy_size);
		bool ProcessLogoff(const int session_index);

		function<void(const int, const void*, const short)> SendPacketFunc;

	private:
		PacketHeader* GetPacketHeader(char* buf);
		PacketBasicRes GetPacketBasicRes(const PacketId id, const ErrorCode error_code);
		PacketBasicEnterLeaveReq* GetPacketBasicEnterLeaveReq(char* buf);


	private:
		using PacketFunc = void(PacketManager::*)(const int, char*, const short);
		std::unordered_map<PacketId, PacketFunc> packet_func_dictionary_;

		UserManager* user_mgr_		= nullptr;
		LobbyManager* lobby_mgr_	= nullptr;
		server_library::ILog* log_	= nullptr;
	};
}