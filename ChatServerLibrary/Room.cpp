//#include "ErrorCode.h"
//#include "Packet.h"
//#include "Room.h"
//#include "User.h"
#include "stdafx.h"

using PacketId = Common::PacketId;
namespace ChatServerLibrary
{
	Room::Room() {}
	Room::~Room() {}

	void Room::Init(const short index, const short maxUserCount, ILog* log)
	{
		Log = log;
		Index = index;
		MaxUserCount = maxUserCount;
	}

	//void Room::Set(IocpServer* server, ILog* logger)
	//{
	//	Server = server;
	//	Log = logger;
	//}

	void Room::Clear()
	{
		Created = false;
		Title		= nullptr;
		UserIndexDic.clear();
	}

	ErrorCode Room::SetRoom(short lobbyIndex, const char* roomTitle)
	{
		if (Created == true) {
			return ErrorCode::ROOM_ALREADY_CREATED;
		}

		Created		= true;
		LobbyIndex	= lobbyIndex;
		Title		= roomTitle;
		return ErrorCode::NONE;
	}

	ErrorCode Room::EnterRoom(User* user)
	{
		if (Created == false) {
			return ErrorCode::ROOM_NOT_CREATED;
		}

		if (UserIndexDic.size() == MaxUserCount) {
			return ErrorCode::ROOM_ENTER_MEMBER_FULL;
		}

		user->EnterRoom(LobbyIndex, Index);
		UserIndexDic.insert({user->GetIndex(), user});

		return ErrorCode::NONE;
	}

	void Room::SendAllUsersInfoToSession(const int sessionIndex)
	{
		Common::PacketNotifyEntrance packet;
		packet.UsersCount = (short)UserIndexDic.size();
		int index = 0;
		short totalSize = 4;
		for (auto userInfo : UserIndexDic) {
			if (userInfo.second == nullptr) {
				continue;
			}

			std::string idString = userInfo.second->GetId();
			short size = (short)idString.size();
			memcpy(&packet.UserId[index], &size, 2);
			index += 2;
			memcpy(&packet.UserId[index], userInfo.second->GetId(), size);
			index += size;
			totalSize += size + 2;
		}
		packet.ErrorCode = (short)ErrorCode::NONE;
		//server_->SetSendingData(sessionIndex, (short)PacketId::ROOM_ENTER_RES, totalSize, (char*)&packet);
	}

	void Room::NotifyToAll(short packetId, const int userIndex)
	{
		if (UserIndexDic.size() <= 1) {
			return;
		}

		Common::PacketNotifyNewUser packet;
		short totalSize = 2;//(sizeof erroCode)

		auto user = FindUser(userIndex);
		std::string idString = user->GetId();
		auto size = idString.size();
		memcpy(packet.UserId, &size, 2);
		memcpy(&packet.UserId[2], user->GetId(), size);
		totalSize += (short)size + 2;
		packet.ErrorCode = (short)ErrorCode::NONE;

		SendToAllUsers(packetId, sizeof(packet), (const char*)&packet, user->GetSessionIndex());
	}

	User* Room::FindUser(const int userIndex)
	{
		auto findIter = UserIndexDic.find(userIndex);
		if (findIter == UserIndexDic.end()) {
			return nullptr;
		}

		return findIter->second;
	}

	bool Room::IsMaster(const short userIndex)
	{
		return UserIndexDic[0]->GetIndex() == userIndex ? true : false;
	}

	ErrorCode Room::LeaveRoom(const short userIndex)
	{
		if (Created == false) {
			return ErrorCode::ROOM_NOT_CREATED;
		}

		auto user = FindUser(userIndex);
		if (user == nullptr) {
			return ErrorCode::ROOM_LEAVE_NOT_MEMBER;
		}

		UserIndexDic.erase(userIndex);
		if (UserIndexDic.empty()) {
			Clear();
		}

		return ErrorCode::NONE;
	}

	void Room::SendToAllUsers(const short packetId, const short dataSize, const char* data, const int exceptionIndex)
	{
		for (auto userInfo : UserIndexDic) {
			if (userInfo.second->GetSessionIndex() == exceptionIndex) {
				continue;
			}

			//server_->SetSendingData(userInfo.second->GetSessionIndex(), packetId, dataSize, data);
		}
	}

	short& Room::GetIndex()
	{ 
		return Index; 	
	}

	bool Room::IsCreated()
	{ 
		return Created; 
	}

	const char* Room::GetTitle() 
	{ 
		return Title; 
	}

	short Room::GetMaxUserCount()
	{ 
		return MaxUserCount; 
	}

	short Room::GetUserCount() 
	{ 
		return (short)UserIndexDic.size(); 
	}

	void Room::SendChat(const char* userId, const short roomIndex, const char* msg)
	{
		Common::PacketRoomChat packetChat;
		std::string temp = userId;
		packetChat.IdSize = (short)temp.size();
		memcpy(packetChat.UserId, userId, packetChat.IdSize);
		temp = msg;
		packetChat.MsgSize = (short)temp.size();
		memcpy(packetChat.Msg, msg, packetChat.MsgSize);

		ChatToAllUsers((short)PacketId::ROOM_CHAT_RES, roomIndex, sizeof packetChat, (const char*)&packetChat);
	}

	void Room::ChatToAllUsers(const short packetId, const short roomIndex, const short dataSize, const char* data)
	{
		for (auto userInfo : UserIndexDic) {
			if (userInfo.second->GetRoomIndex() != roomIndex) {
				continue;
			}

			//server_->SetSendingData(userInfo.second->GetSessionIndex(), packetId, dataSize, data);
		}
	}
}