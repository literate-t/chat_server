#pragma once
#include "stdafx.h"
//#include <vector>
//#include <unordered_map>

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
	class User;
	class Room;
	using IocpServer = ServerLibrary::IocpServer;
	using ILog = ServerLibrary::ILog;
	using ErrorCode = Common::ErrorCode;

	struct LobbyUser
	{
		short Index	= -1;
		User* User	= nullptr;
	};

	class Lobby
	{
	public:
		Lobby();
		~Lobby();

		void Init(const short lobbyIndex, const short maxLobbyUserCount, const short maxRoomCount, const short maxRoomUserCount, ILog* log);
		void Release();
		//void Set(IocpServer* server, ILog* logger);
		short& GetIndex();
		
		ErrorCode EnterLobby(User* user);
		void SendAllUsersInfoToSession(short packetId, const int sessionIndex);
		void NotifyToAll(short packetId, const int userIndex);
		ErrorCode LeaveLobby(const int userIndex);
		ErrorCode LeaveLobbyToEnterRomm(const int userIndex);

		size_t GetUserCount();
		Room* CreateRoom(short index);
		Room* GetRoom(const short roomIndex);

		short& GetMaxUserCount();
		size_t GetMaxRoomCount();

		function<void(int, void*, short)> SendPacketFunc;

	private:
		void SendToAllUsers(const short packetId, const short dataSize, const char* msg, const int exceptionIndex = -1);
		User* FindUser(const int userIndex);
		ErrorCode AddUser(User* user);
		void RemoveUser(const int userIndex);

	private:
		ILog* Log	= nullptr;
		IocpServer* Server = nullptr;

		short LobbyIndex = -1;
		short MaxUserCount = -1;

		std::vector<LobbyUser>					LobbyUserList;
		std::unordered_map<int, User*>			UserIndexDic;
		std::unordered_map<const char*, User*>	UserIdDic;
		std::vector<Room*>						RoomList;
	};
}