//#include "ErrorCode.h"
//#include "Lobby.h"
//#include "Packet.h"
//#include "Room.h"
//#include "User.h"
//#include <algorithm>
#include "stdafx.h"

using PacketId = Common::PacketId;
namespace ChatServerLibrary
{
	Lobby::Lobby() {}
	Lobby::~Lobby() {}

	void Lobby::Init(const short lobbyIndex, const short max_lobbyUserCount, const short max_roomCount, const short max_roomUserCount)
	{
		LobbyIndex = lobbyIndex;
		MaxUserCount = max_lobbyUserCount;

		for (int i = 0; i < MaxUserCount; ++i) {
			LobbyUser lobbyUser;
			lobbyUser.Index = (short)i;
			LobbyUserList.push_back(lobbyUser);
		}

		for (int i = 0; i < max_roomCount; ++i) {
			RoomList.emplace_back(new Room());
			RoomList[i]->Init((short)i, max_roomUserCount);
		}
	}

	void Lobby::Release()
	{
		for (size_t i = 0; i < RoomList.size(); ++i) {
			delete RoomList[i];
		}

		RoomList.clear();
	}

	void Lobby::Set(IocpServer* server, ILog* log)
	{
		Server = server;
		Log = log;

		for (auto& room : RoomList) {
			room->Set(Server, Log);
		}
	}

	ErrorCode Lobby::AddUser(User* user)
	{
		auto findIter = std::find_if(std::begin(LobbyUserList), std::end(LobbyUserList),
			[](auto& lobbyUser) {return lobbyUser.User == nullptr; });

		if (findIter == std::end(LobbyUserList)) {
			return ErrorCode::LOBBY_ENTER_MAX_USER_COUNT;
		}
		
		findIter->User = user;
		return ErrorCode::NONE;
	}

	ErrorCode Lobby::EnterLobby(User* user)
	{
		if (UserIndexDic.size() >= MaxUserCount) {
			return ErrorCode::LOBBY_ENTER_MAX_USER_COUNT;
		}

		if (FindUser(user->GetIndex()) != nullptr) {
			return ErrorCode::LOBBY_ENTER_USER_DUPLICATION;
		}

		auto add_result = AddUser(user);
		if (add_result != ErrorCode::NONE) {
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
		if (user == nullptr) {
			ErrorCode::LOBBY_LEAVE_USER_INVALID;
		}

		RemoveUser(userIndex);
		user->LeaveLobby();

		UserIndexDic.erase(userIndex);
		UserIdDic.erase(user->GetId());

		return ErrorCode::NONE;
	}

	ErrorCode Lobby::LeaveLobbyToEnterRomm(const int userIndex)
	{
		auto user = FindUser(userIndex);
		if (user == nullptr) {
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
		if (findIter == UserIndexDic.end()) {
			return nullptr;
		}

		return findIter->second;
	}

	void Lobby::SendAllUsersInfoToSession(short packetId, const int sessionIndex)
	{
		Common::PacketNotifyEntrance packet;
		packet.UsersCount = (short)UserIndexDic.size();
		int index = 0;
		short total_size = 4;
		for (auto user : UserIdDic) {
			if (user.first == nullptr) {
				continue;
			}

			std::string id_string = user.first;
			short size = (short)id_string.size();
			memcpy(&packet.UserId[index], &size, 2);
			index += 2;
			memcpy(&packet.UserId[index], user.first, size);
			index += size;
			total_size += size + 2;
		}
		packet.ErrorCode = (short)ErrorCode::NONE;
		//Server->SetSendingData(sessionIndex, packetId, total_size/*sizeof(packet)*/, (char*)&packet);
	}

	void Lobby::NotifyToAll(short packetId, const int userIndex)
	{
		if (UserIndexDic.size() == 1) {
			return;
		}

		Common::PacketNotifyNewUser packet;
		short total_size = 2;	//(sizeof erroCode)
		auto user = FindUser(userIndex);
		std::string id_string = user->GetId();
		auto size = id_string.size();
		memcpy(packet.UserId, &size, 2);
		memcpy(&packet.UserId[2], user->GetId(), size);
		total_size += (short)size + 2;
		packet.ErrorCode = (short)ErrorCode::NONE;

		SendToAllUsers(packetId, total_size, (const char*)&packet, user->GetSessionIndex());
	}

	void Lobby::SendToAllUsers(const short packetId, const short data_size, const char* data, const int exception_sessionIndex)
	{
		for (auto& user : UserIndexDic) {
			if (user.second->GetSessionIndex() == exception_sessionIndex) {
				continue;
			}

			//Server->SetSendingData(user.second->GetSessionIndex(), packetId, data_size, data);
		}
	}

	void Lobby::RemoveUser(const int userIndex)
	{
		auto findIter = std::find_if(std::begin(LobbyUserList), std::end(LobbyUserList), 
			[&userIndex](auto& lobbyUser) { return lobbyUser.User != nullptr && lobbyUser.User->GetIndex() == userIndex; });

		if (findIter == std::end(LobbyUserList)) {
			return;
		}

		findIter->User = nullptr;
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
		if (roomIndex > RoomList.size() - 1) {
			nullptr;
		}

		if (RoomList[roomIndex]->IsCreated() == false) {
			return RoomList[roomIndex];
		}
		
		return GetRoom(roomIndex);
	}

	Room* Lobby::GetRoom(const short roomIndex)
	{
		if (roomIndex < 0 || roomIndex >= RoomList.size() - 1) {
			return nullptr;
		}

		else if (RoomList[roomIndex]->IsCreated() == true) {
			return RoomList[roomIndex];
		}

		else {
			return nullptr;
		}
	}
}