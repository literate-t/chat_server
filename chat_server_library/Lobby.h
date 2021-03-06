#pragma once
#include "stdafx.h"

//namespace server_library
//{
//	class IocpServer;
//	class ILog;
//}
//
//namespace Common
//{
//	enum class ErrorCode : short;
//}

namespace chat_server_library
{
	class User;
	class Room;
	using IocpServer = server_library::IocpServer;
	using ILog = server_library::ILog;
	using ErrorCode = Common::ErrorCode;
	using PacketId = Common::PacketId;

	struct LobbyUser
	{
		short index_ = -1;
		User* user_	 = nullptr;
	};

	class Lobby
	{
	public:
		Lobby();
		~Lobby();

		void Init(const short lobby_index, const short max_lobby_user_count, const short max_room_count, const short max_room_user_count, ILog* log);
		void Release();
		short GetIndex() const;
		
		ErrorCode EnterLobby(User* user);
		void SendAllUsersInfoToSession(const PacketId packet_id, const int session_index);
		void NotifyToAll(const PacketId packet_id, const int user_index);
		ErrorCode LeaveLobby(const int user_index);
		ErrorCode LeaveLobbyToEnterRoom(const int user_index);

		size_t GetUserCount() const;
		Room* CreateRoom(const short index);
		Room* GetRoom(const short room_index) const;

		short GetMaxUserCount() const;
		size_t GetMaxRoomCount() const;

		function<void(const int, const void*, const short)> SendPacketFunc;

	private:
		void SendToAllUsers(const void* packet, const short packet_size, const int exception_index = -1);
		User* FindUser(const int user_index);
		ErrorCode AddUser(User* user);
		void RemoveUser(const int user_index);

	private:
		ILog* log_	= nullptr;
		IocpServer* server_ = nullptr;

		short lobby_index_ = -1;
		short max_user_count_ = -1;

		std::vector<LobbyUser>					lobby_user_list_;
		std::unordered_map<int, User*>	user_index_dic_;
		std::unordered_map<const char*, User*>	user_id_dic_;
		std::vector<Room*>						room_list_;
	};
}