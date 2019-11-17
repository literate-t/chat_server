#pragma once
//#include <unordered_map>
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

		void Init(const short index, const short maxUserCount);
		void Set(IocpServer* server, ILog* log);
		void Clear();
		short& GetIndex();
		bool IsCreated();
		const char* GetTitle();
		short GetMaxUserCount();
		short GetUserCount();
		User* FindUser(const int userIndex);

		ErrorCode SetRoom(short lobbyIndex, const char* roomTitle);
		ErrorCode EnterRoom(User* user);
		void SendAllUsersInfoToSession(const int sessionIndex);
		void NotifyToAll(short packetId, const int userIndex);
		ErrorCode LeaveRoom(const short user_index);

		bool IsMaster(const short userIndex);

		void SendToAllUsers(const short packetId, const short dataSize, const char* data, const int exceptionIndex = -1);
		void SendChat(const char* userId, const short roomIndex, const char* msg);
		void ChatToAllUsers(const short packetId, const short roomIndex, const short dataSize, const char* data);

	private:
		ILog* Log = nullptr;
		IocpServer* Server = nullptr;

		short Index = -1;
		short LobbyIndex = -1;
		short MaxUserCount = 0;
		bool Created = false;

		const char* Title = nullptr;
		std::unordered_map<int, User*> UserIndexDic;
	};
}