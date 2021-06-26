#include "stdafx.h"

using PacketId = Common::PacketId;
namespace chat_server_library
{
	Room::Room() {}
	Room::~Room() {}

	void Room::Init(const short index, const short max_user_count, ILog* log)
	{
		log_			= log;
		index_			= index;
		max_user_count_ = max_user_count;
	}

	void Room::Clear()
	{
		created_	= false;
		title_		= nullptr;
		user_index_dic_.clear();
	}

	ErrorCode Room::SetRoom(short lobby_index, const char* room_title)
	{
		if (true == created_) 
		{
			return ErrorCode::ROOM_ALREADY_CREATED;
		}
		created_		= true;
		lobby_index_	= lobby_index;
		title_			= room_title;
		return ErrorCode::NONE;
	}

	ErrorCode Room::EnterRoom(User* user)
	{
		if (false == created_) {
			return ErrorCode::ROOM_NOT_CREATED;
		}

		if (user_index_dic_.size() == max_user_count_) {
			return ErrorCode::ROOM_ENTER_MEMBER_FULL;
		}

		user->EnterRoom(lobby_index_, index_);
		user_index_dic_.insert({user->GetIndex(), user});

		return ErrorCode::NONE;
	}

	void Room::SendAllUsersInfoToSession(short packet_id, const int session_index)
	{
		PacketNotifyEntrance packet;
		packet.users_count_ = (short)user_index_dic_.size();
		int index = 0;
		short total_size = 4; // UserCount(short) + ErrorCode(short)
		for (auto user_info : user_index_dic_) 
		{
			if (nullptr == user_info.second)
			{
				continue;
			}

			std::string id_string = user_info.second->GetId();
			short size = (short)id_string.size();
			memcpy(&packet.user_id_[index], &size, 2);
			index += 2;
			memcpy(&packet.user_id_[index], user_info.second->GetId(), size);
			index += size;
			total_size += size + 2;
		}
		packet.total_size_ = total_size + kPacketHeaderLength;
		packet.id_ = packet_id;
		packet.error_code_ = (short)ErrorCode::NONE;
		SendPacketFunc(session_index, &packet, packet.total_size_);
	}

	void Room::NotifyToAll(short packet_id, const int user_index)
	{
		if (user_index_dic_.size() <= 1) 
		{
			return;
		}

		PacketNotifyNewUser packet;
		short total_size = 2;//(sizeof erroCode)

		auto user = FindUser(user_index);
		std::string id_string = user->GetId();
		auto size = id_string.size();
		memcpy(packet.user_id_, &size, 2);
		memcpy(&packet.user_id_[2], user->GetId(), size);
		total_size += (short)size + 2;

		packet.total_size_ = total_size + kPacketHeaderLength;
		packet.id_ = packet_id;
		packet.error_code_ = static_cast<short>(ErrorCode::NONE);
		SendToAllUsers(&packet, packet.total_size_, user->GetSessionIndex());
	}

	void Room::SendToAllUsers(void* packet, const short packet_size, const int exception_index)
	{
		for (auto& user : user_index_dic_)
		{
			if (user.second->GetSessionIndex() == exception_index)
			{
				continue;
			}
			SendPacketFunc(user.second->GetSessionIndex(), packet, packet_size);
		}
	}

	User* Room::FindUser(const int user_index)
	{
		auto findIter = user_index_dic_.find(user_index);
		if (findIter == user_index_dic_.end()) {
			return nullptr;
		}

		return findIter->second;
	}

	bool Room::IsMaster(const short user_index)
	{
		return user_index_dic_[0]->GetIndex() == user_index ? true : false;
	}

	ErrorCode Room::LeaveRoom(const short user_index)
	{
		if (false == created_) {
			return ErrorCode::ROOM_NOT_CREATED;
		}

		auto user = FindUser(user_index);
		if (nullptr == user) {
			return ErrorCode::ROOM_LEAVE_NOT_MEMBER;
		}
		
		user_index_dic_.erase(user_index);
		if (user_index_dic_.empty()) 
		{
			Clear();
		}

		return ErrorCode::NONE;
	}

	short& Room::GetIndex()
	{ 
		return index_; 	
	}

	bool Room::IsCreated()
	{ 
		return created_; 
	}

	const char* Room::GetTitle() 
	{ 
		return title_; 
	}

	short Room::GetMaxUserCount()
	{ 
		return max_user_count_; 
	}

	short Room::GetUserCount() 
	{ 
		return (short)user_index_dic_.size(); 
	}

	void Room::SendChat(const char* user_id, const short room_index, const char* msg)
	{
		PacketRoomChat packet_chat;
		std::string temp = user_id;
		packet_chat.id_size_ = (short)temp.size();
		memcpy(packet_chat.user_id_, user_id, packet_chat.id_size_);

		temp = msg;
		packet_chat.msg_size_ = (short)temp.size();
		memcpy(packet_chat.msg_, msg, packet_chat.msg_size_);

		packet_chat.id_ = static_cast<short>(PacketId::ROOM_CHAT_RES);
		packet_chat.total_size_ = sizeof PacketRoomChat;
		packet_chat.error_code_ = static_cast<short>(ErrorCode::NONE);

		ChatToAllUsers(&packet_chat, packet_chat.total_size_, room_index);
	}

	void Room::ChatToAllUsers(void* packet, const short packet_size, const int room_index)
	{
		for (auto user : user_index_dic_)
		{
			if (user.second->GetRoomIndex() != room_index) 
			{
				continue;
			}
			SendPacketFunc(user.second->GetSessionIndex(), packet, packet_size);
		}
	}
}