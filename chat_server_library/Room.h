#pragma once
#include "stdafx.h"

namespace server_library
{
	class IocpServer;
	class ILog;
}

namespace Common
{
	enum class ErrorCode : short;
}

namespace chat_server_library
{
	using IocpServer = server_library::IocpServer;
	using ILog		 = server_library::ILog;
	using ErrorCode  = Common::ErrorCode;
	using PacketId = Common::PacketId;
	class User;

	class Room
	{
	public:
		Room() = default;
		~Room() = default;

		void Init(const short index, const short max_user_count, ILog* log);
		void Clear();
		short GetIndex() const;
		bool IsCreated() const;
		const char* GetTitle() const;
		short GetMaxUserCount() const;
		size_t GetUserCount() const;
		User* FindUser(const int user_index) const;

		ErrorCode SetRoom(const short lobby_index, const char* room_title);
		ErrorCode EnterRoom(User* user);
		void SendAllUsersInfoToSession(const PacketId packet_id, const int session_index);
		void NotifyToAll(PacketId packet_id, const int user_index);
		ErrorCode LeaveRoom(const short user_index);

		bool IsMaster(const short user_index);

		void SendToAllUsers(const void* packet, const short packet_size, const int exception_index = -1);
		void SendChat(const char* user_id, const short room_index, const char* msg);
		void ChatToAllUsers(const void* packet, const short packet_size, const int room_index);

		function<void(const int, const void*, const short)> SendPacketFunc;

	private:
		ILog* log_ = nullptr;

		short index_ = -1;
		short lobby_index_ = -1;
		short max_user_count_ = 0;
		bool created_ = false;

		const char* title_ = nullptr;
		std::unordered_map<int, User*> user_index_dic_;
	};
}