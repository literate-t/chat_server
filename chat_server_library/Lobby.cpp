#include "stdafx.h"

using PacketId = Common::PacketId;
namespace chat_server_library
{
	Lobby::Lobby() {}
	Lobby::~Lobby() {}

	void Lobby::Init(const short lobby_index, const short max_lobby_user_count, const short max_room_count, const short max_room_user_count, ILog* log)
	{
		lobby_index_ = lobby_index;
		max_user_count_ = max_lobby_user_count;
		log_ = log;

		for (int i = 0; i < max_user_count_; ++i) 
		{
			LobbyUser lobby_user;
			lobby_user.index_ = (short)i;
			lobby_user_list_.push_back(lobby_user);
		}

		for (int i = 0; i < max_room_count; ++i) 
		{
			room_list_.emplace_back(new Room());
			room_list_[i]->Init((short)i, max_room_user_count, log_);
			room_list_[i]->SendPacketFunc = SendPacketFunc;
		}
	}

	void Lobby::Release()
	{
		for (size_t i = 0; i < room_list_.size(); ++i) 
		{
			delete room_list_[i];
		}

		room_list_.clear();
	}

	ErrorCode Lobby::AddUser(User* user)
	{
		auto findIter = std::find_if(std::begin(lobby_user_list_), std::end(lobby_user_list_),
			[](auto& lobby_user)
			{
				return lobby_user.user_ == nullptr; 
			});

		if (findIter == std::end(lobby_user_list_)) 
		{
			return ErrorCode::LOBBY_ENTER_MAX_USER_COUNT;
		}
		
		findIter->user_ = user;
		return ErrorCode::NONE;
	}

	ErrorCode Lobby::EnterLobby(User* user)
	{
		if (user_index_dic_.size() >= max_user_count_) 
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

		user->EnterLobby(lobby_index_);
		user_index_dic_.insert({user->GetIndex(), user});
		user_id_dic_.insert({user->GetId(), user});

		return ErrorCode::NONE;
	}

	ErrorCode Lobby::LeaveLobby(const int user_index)
	{
		auto user = FindUser(user_index);
		if (user == nullptr) 
		{
			ErrorCode::LOBBY_LEAVE_USER_INVALID;
		}

		RemoveUser(user_index);
		user->LeaveLobby();

		user_index_dic_.erase(user_index);
		user_id_dic_.erase(user->GetId());

		return ErrorCode::NONE;
	}

	void Lobby::RemoveUser(const int user_index)
	{
		auto findIter = std::find_if(std::begin(lobby_user_list_), std::end(lobby_user_list_),
			[&user_index](auto& lobby_user)
			{
				return lobby_user.user_ != nullptr && lobby_user.user_->GetIndex() == user_index;
			});

		if (findIter == std::end(lobby_user_list_))
		{
			return;
		}

		findIter->user_ = nullptr;
	}

	ErrorCode Lobby::LeaveLobbyToEnterRomm(const int user_index)
	{
		auto user = FindUser(user_index);
		if (user == nullptr) 
		{
			ErrorCode::LOBBY_LEAVE_USER_INVALID;
		}

		RemoveUser(user_index);
		user->LeaveLobbyToEnterRoom();

		user_index_dic_.erase(user_index);
		user_id_dic_.erase(user->GetId());

		return ErrorCode::NONE;
	}

	User* Lobby::FindUser(const int user_index)
	{
		auto findIter = user_index_dic_.find(user_index);
		if (findIter == user_index_dic_.end()) 
		{
			return nullptr;
		}

		return findIter->second;
	}

	void Lobby::SendAllUsersInfoToSession(short packet_id, const int session_index)
	{
		PacketNotifyEntrance packet;
		packet.users_count_ = (short)user_index_dic_.size();
		int index = 0;
		short total_size = 4; // UserCount(short) + ErroCode(short)
		for (auto user : user_id_dic_) 
		{
			if (user.first == nullptr) 
			{
				continue;
			}

			std::string id_string = user.first;
			short size = (short)id_string.size();
			memcpy(&packet.user_id_[index], &size, 2);
			index += 2;
			memcpy(&packet.user_id_[index], user.first, size);
			index += size;
			total_size += size + 2;
		}
		packet.total_size_ = total_size + kPacketHeaderLength;
		packet.id_ = packet_id;
		packet.error_code_ = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(session_index, &packet, packet.total_size_);
	}

	void Lobby::NotifyToAll(short packet_id, const int user_index)
	{
		if (user_index_dic_.size() == 1) 
		{
			return;
		}

		PacketNotifyNewUser packet;
		short total_size = 2;	//(sizeof error_code)
		auto user = FindUser(user_index);
		std::string id_string = user->GetId();
		auto size = id_string.size();
		memcpy(packet.user_id_, &size, 2);
		memcpy(&packet.user_id_[2], user->GetId(), size);
		total_size += (short)size + 2;

		packet.id_			= packet_id;
		packet.total_size_	= total_size + kPacketHeaderLength;
		packet.error_code_ = static_cast<short>(ErrorCode::NONE);
		SendToAllUsers(&packet, packet.total_size_, user->GetSessionIndex());
	}

	void Lobby::SendToAllUsers(void* packet, const short packet_size, const int exception_index)
	{
		for (auto& user : user_index_dic_) {
			if (user.second->GetSessionIndex() == exception_index) 
			{
				continue;
			}
			SendPacketFunc(user.second->GetSessionIndex(), packet, packet_size);
		}
	}

	short& Lobby::GetIndex()
	{ 
		return lobby_index_; 
	}

	size_t Lobby::GetUserCount()
	{
		return user_index_dic_.size();
	}

	short& Lobby::GetMaxUserCount()
	{ 
		return max_user_count_; 
	}

	size_t Lobby::GetMaxRoomCount()
	{ 
		return room_list_.size(); 
	}

	Room* Lobby::CreateRoom(short room_index)
	{
		if (room_index > room_list_.size() - 1) 
		{
			nullptr;
		}

		if (room_list_[room_index]->IsCreated() == false) 
		{
			return room_list_[room_index];
		}
		
		return GetRoom(room_index);
	}

	Room* Lobby::GetRoom(const short room_index)
	{
		if (room_index < 0 || room_index >= room_list_.size() - 1) 
		{
			return nullptr;
		}

		else if (room_list_[room_index]->IsCreated() == true) 
		{
			return room_list_[room_index];
		}

		else 
		{
			return nullptr;
		}
	}
}