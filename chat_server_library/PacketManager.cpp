#include "stdafx.h"

namespace chat_server_library
{
	void PacketManager::Init(UserManager* user_manager, LobbyManager* lobby_manager, server_library::ILog* log)
	{
		packet_func_dictionary_[PacketId::LOGIN_REQ] = &PacketManager::Login;
		packet_func_dictionary_[PacketId::LOBBY_ENTER_REQ]	= &PacketManager::LobbyEnter;
		packet_func_dictionary_[PacketId::LOBBY_LEAVE_REQ]	= &PacketManager::LobbyLeave;
		packet_func_dictionary_[PacketId::ROOM_ENTER_REQ]	= &PacketManager::RoomEnter;
		packet_func_dictionary_[PacketId::ROOM_LEAVE_REQ]	= &PacketManager::RoomLeave;
		packet_func_dictionary_[PacketId::ROOM_CHAT_REQ]	= &PacketManager::RoomChat;

		user_mgr_ = user_manager;
		lobby_mgr_ = lobby_manager;
		log_ = log;
	}

	void PacketManager::ProcessPacket(int session_index, char* buf, const short copy_size)
	{
		PacketHeader* header = GetPacketHeader(buf);
		auto iter = packet_func_dictionary_.find(header->id_);
		if (iter != packet_func_dictionary_.end())
		{
			(this->*(iter->second))(session_index, buf, copy_size);
		}
	}

	void PacketManager::Login(const int session_index, char* buf, const short copy_size)
	{
		if (copy_size - static_cast<short>(kPacketHeaderLength) != static_cast<short>(kLoginReqPacketSize))
		{
			log_->Write(server_library::LogType::L_ERROR, "Login packet size error");
			return;
		}

		auto login_req = reinterpret_cast<PacketLoginReq*>(&buf[kPacketHeaderLength]);		
		log_->Write(server_library::LogType::L_INFO, "id_:%s", login_req->id_);
		auto result = user_mgr_->AddUser(session_index, login_req->id_);

		if (ErrorCode::NONE != result)
		{
			PacketBasicRes packet_res = 
				GetPacketBasicRes(PacketId::LOGIN_RES, result);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}
		PacketBasicRes packet_res =
			GetPacketBasicRes(PacketId::LOGIN_RES, ErrorCode::NONE);
		SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
	}

	bool PacketManager::ProcessLogoff(const int session_index)
	{
		auto [error_code, user] = user_mgr_->GetUser(session_index);
		if (ErrorCode::NONE != error_code)
		{
			log_->Write(server_library::LogType::L_INFO, "No user");
			return false;
		}
		
		if (true == user->IsDomainRoom())
		{
			auto lobby = lobby_mgr_->GetLobby(user->GetLobbyIndex());
			if (nullptr != lobby)
			{
				auto room = lobby->GetRoom(user->GetRoomIndex());
				if (nullptr != room)
				{
					// 룸에 남아 있는 세션에게 퇴장하는 세션을 알림
					room->NotifyToAll(PacketId::ROOM_LEAVE_USER_NTF, user->GetIndex());

					// 방 퇴장 처리
					room->LeaveRoom(user->GetIndex());
				}
			}
		}
		else if (true == user->IsDomainLobby())
		{
			auto lobby = lobby_mgr_->GetLobby(user->GetLobbyIndex());
			if (nullptr != lobby)
			{
				// 로비 퇴장 처리
				lobby->NotifyToAll(PacketId::LOBBY_LEAVE_USER_NTF, user->GetIndex());
				lobby->LeaveLobby(user->GetIndex());
			}
		}

		auto result = user_mgr_->RemoveUser(session_index);
		if (ErrorCode::NONE == result)
		{
			return true;
		}
		return false;
	}

	PacketHeader* PacketManager::GetPacketHeader(char* buf)
	{
		return reinterpret_cast<PacketHeader*>(buf);
	}

	PacketBasicRes PacketManager::GetPacketBasicRes(PacketId id, ErrorCode error_code)
	{
		return PacketBasicRes(id, error_code);
	}
}