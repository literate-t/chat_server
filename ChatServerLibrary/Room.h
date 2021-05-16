#pragma once
#include "stdafx.h"

namespace ServerLibrary
{
	class IocpServer;
	class ILog;
}

namespace Common
{
	enum class ErrorCode : short;
}

namespace ChatServerLibrary
{
	using IocpServer = ServerLibrary::IocpServer;
	using ILog		 = ServerLibrary::ILog;
	using ErrorCode  = Common::ErrorCode;
	class User;

	class Room
	{
	public:
		Room();
		~Room();

		void Init(const short index, const short max_user_count, ILog* log);
		void Clear();
		short& GetIndex();
		bool IsCreated();
		const char* GetTitle();
		short GetMaxUserCount();
		short GetUserCount();
		User* FindUser(const int user_index);

		ErrorCode SetRoom(short lobby_index, const char* room_title);
		ErrorCode EnterRoom(User* user);
		void SendAllUsersInfoToSession(short packet_id, const int session_index);
		void NotifyToAll(short packet_id, const int user_index);
		ErrorCode LeaveRoom(const short user_index);

		bool IsMaster(const short user_index);

		void SendToAllUsers(void* packet, const short packet_size, const int exception_index = -1);
		void SendChat(const char* user_id, const short room_index, const char* msg);
		void ChatToAllUsers(void* packet, const short packet_size, const int room_index);

		function<void(int, void*, short)> SendPacketFunc;

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