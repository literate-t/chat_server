#include "stdafx.h"

using PacketId = Common::PacketId;
namespace ChatServerLibrary
{
	Lobby::Lobby() {}
	Lobby::~Lobby() {}

	void Lobby::Init(const short lobbyIndex, const short maxLobbyUserCount, const short maxRoomCount, const short maxRoomUserCount, ILog* log)
	{
		LobbyIndex = lobbyIndex;
		MaxUserCount = maxLobbyUserCount;
		Log = log;

		for (int i = 0; i < MaxUserCount; ++i) 
		{
			LobbyUser lobbyUser;
			lobbyUser.Index = (short)i;
			LobbyUserList.push_back(lobbyUser);
		}

		for (int i = 0; i < maxRoomCount; ++i) 
		{
			RoomList.emplace_back(new Room());
			RoomList[i]->Init((short)i, maxRoomUserCount, Log);
			RoomList[i]->SendPacketFunc = SendPacketFunc;
		}
	}

	void Lobby::Release()
	{
		for (size_t i = 0; i < RoomList.size(); ++i) 
		{
			delete RoomList[i];
		}

		RoomList.clear();
	}

	ErrorCode Lobby::AddUser(User* user)
	{
		auto findIter = std::find_if(std::begin(LobbyUserList), std::end(LobbyUserList),
			[](auto& lobbyUser)
			{
				return lobbyUser.User == nullptr; 
			});

		if (findIter == std::end(LobbyUserList)) 
		{
			return ErrorCode::LOBBY_ENTER_MAX_USER_COUNT;
		}
		
		findIter->User = user;
		return ErrorCode::NONE;
	}

	ErrorCode Lobby::EnterLobby(User* user)
	{
		if (UserIndexDic.size() >= MaxUserCount) 
		{
			return ErrorCode::LOBBY_ENTER_MAX_USER_COUNT;
		}

		if (FindUser(user->GetIndex()) != nullptr) 
		{
			return ErrorCode::LOBBY_ENTER_USER_DUPLICATION;
		}

		auto add_result = AddUser(user);
		if (add_result != ErrorCode::NONE) 
		{
			return add_result;
		}

		user->EnterLobby(LobbyIndex);
		UserIndexDic.insert({user->GetIndex(), user});
		UserIdDic.insert({user->GetId(), user});

		return ErrorCode::NONE;
	}

	ErrorCode Lobby::LeaveLobby(const int userIndex)
	{
		auto user = FindUser(userIndex);
		if (user == nullptr) 
		{
			ErrorCode::LOBBY_LEAVE_USER_INVALID;
		}

		RemoveUser(userIndex);
		user->LeaveLobby();

		UserIndexDic.erase(userIndex);
		UserIdDic.erase(user->GetId());

		return ErrorCode::NONE;
	}

	void Lobby::RemoveUser(const int userIndex)
	{
		auto findIter = std::find_if(std::begin(LobbyUserList), std::end(LobbyUserList),
			[&userIndex](auto& lobbyUser)
			{
				return lobbyUser.User != nullptr && lobbyUser.User->GetIndex() == userIndex;
			});

		if (findIter == std::end(LobbyUserList))
		{
			return;
		}

		findIter->User = nullptr;
	}

	ErrorCode Lobby::LeaveLobbyToEnterRomm(const int userIndex)
	{
		auto user = FindUser(userIndex);
		if (user == nullptr) 
		{
			ErrorCode::LOBBY_LEAVE_USER_INVALID;
		}

		RemoveUser(userIndex);
		user->LeaveLobbyToEnterRoom();

		UserIndexDic.erase(userIndex);
		UserIdDic.erase(user->GetId());

		return ErrorCode::NONE;
	}

	User* Lobby::FindUser(const int userIndex)
	{
		auto findIter = UserIndexDic.find(userIndex);
		if (findIter == UserIndexDic.end()) 
		{
			return nullptr;
		}

		return findIter->second;
	}

	void Lobby::SendAllUsersInfoToSession(short packetId, const int sessionIndex)
	{
		PacketNotifyEntrance packet;
		packet.UsersCount = (short)UserIndexDic.size();
		int index = 0;
		short totalSize = 4; // UserCount(short) + ErroCode(short)
		for (auto user : UserIdDic) 
		{
			if (user.first == nullptr) 
			{
				continue;
			}

			std::string id_string = user.first;
			short size = (short)id_string.size();
			memcpy(&packet.UserId[index], &size, 2);
			index += 2;
			memcpy(&packet.UserId[index], user.first, size);
			index += size;
			totalSize += size + 2;
		}
		packet.TotalSize = totalSize + kPacketHeaderLength;
		packet.Id = packetId;
		packet.ErrorCode = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(sessionIndex, &packet, packet.TotalSize);
	}

	void Lobby::NotifyToAll(short packetId, const int userIndex)
	{
		if (UserIndexDic.size() == 1) 
		{
			return;
		}

		PacketNotifyNewUser packet;
		short totalSize = 2;	//(sizeof errorCode)
		auto user = FindUser(userIndex);
		std::string id_string = user->GetId();
		auto size = id_string.size();
		memcpy(packet.UserId, &size, 2);
		memcpy(&packet.UserId[2], user->GetId(), size);
		totalSize += (short)size + 2;

		packet.Id			= packetId;
		packet.TotalSize	= totalSize + kPacketHeaderLength;
		packet.ErrorCode = static_cast<short>(ErrorCode::NONE);
		SendToAllUsers(&packet, packet.TotalSize, user->GetSessionIndex());
	}

	void Lobby::SendToAllUsers(void* packet, const short packetSize, const int exceptionIndex)
	{
		for (auto& user : UserIndexDic) {
			if (user.second->GetSessionIndex() == exceptionIndex) 
			{
				continue;
			}
			SendPacketFunc(user.second->GetSessionIndex(), packet, packetSize);
		}
	}

	short& Lobby::GetIndex()
	{ 
		return LobbyIndex; 
	}

	size_t Lobby::GetUserCount()
	{
		return UserIndexDic.size();
	}

	short& Lobby::GetMaxUserCount()
	{ 
		return MaxUserCount; 
	}

	size_t Lobby::GetMaxRoomCount()
	{ 
		return RoomList.size(); 
	}

	Room* Lobby::CreateRoom(short roomIndex)
	{
		if (roomIndex > RoomList.size() - 1) 
		{
			nullptr;
		}

		if (RoomList[roomIndex]->IsCreated() == false) 
		{
			return RoomList[roomIndex];
		}
		
		return GetRoom(roomIndex);
	}

	Room* Lobby::GetRoom(const short roomIndex)
	{
		if (roomIndex < 0 || roomIndex >= RoomList.size() - 1) 
		{
			return nullptr;
		}

		else if (RoomList[roomIndex]->IsCreated() == true) 
		{
			return RoomList[roomIndex];
		}

		else 
		{
			return nullptr;
		}
	}
}