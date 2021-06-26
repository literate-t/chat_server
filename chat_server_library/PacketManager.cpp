#include "stdafx.h"

namespace chat_server_library
{
	void PacketManager::Init(UserManager* user_manager, LobbyManager* lobby_manager, server_library::ILog* log)
	{
		packet_func_dictionary_[static_cast<short>(PacketId::LOGIN_REQ)] = &PacketManager::Login;
		packet_func_dictionary_[static_cast<short>(PacketId::LOBBY_ENTER_REQ)] = &PacketManager::LobbyEnter;
		packet_func_dictionary_[static_cast<short>(PacketId::LOBBY_LEAVE_REQ)] = &PacketManager::LobbyLeave;
		packet_func_dictionary_[static_cast<short>(PacketId::ROOM_ENTER_REQ)]	 = &PacketManager::RoomEnter;
		packet_func_dictionary_[static_cast<short>(PacketId::ROOM_LEAVE_REQ)] = &PacketManager::RoomLeave;
		packet_func_dictionary_[static_cast<short>(PacketId::ROOM_CHAT_REQ)] = &PacketManager::RoomChat;

		user_mgr_ = user_manager;
		lobby_mgr_ = lobby_manager;
		log_ = log;
	}

	void PacketManager::ProcessPacket(int session_index, char* buf, short copy_size)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buf);
		auto iter = packet_func_dictionary_.find(header->id_);
		if (iter != packet_func_dictionary_.end())
		{
			(this->*(iter->second))(session_index, buf, copy_size);
		}
	}

	void PacketManager::Login(const int session_index, char* buf, short copy_size)
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
			PacketBasicRes packet_res;
			packet_res.id_ = static_cast<short>(PacketId::LOGIN_RES);
			packet_res.error_code_ = static_cast<short>(result);
			SendPacketFunc(session_index, &packet_res, sizeof packet_res);
			return;
		}
		PacketBasicRes packet_res;
		packet_res.id_ = static_cast<short>(PacketId::LOGIN_RES);
		packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
	}

	bool PacketManager::ProcessLogoff(const int session_index)
	{
		auto [error_code, user] = user_mgr_->GetUser(session_index);
		if (ErrorCode::NONE != error_code)
		{
			log_->Write(server_library::LogType::L_INFO, "No user");
		}
		
		if (true == user->IsDomainRoom())
		{
			PacketBasicRes packet_res;

			auto lobby = lobby_mgr_->GetLobby(user->GetLobbyIndex());
			auto room = lobby->GetRoom(user->GetRoomIndex());
			if (nullptr == room)
			{
				packet_res.error_code_ = (short)ErrorCode::ROOM_LEAVE_INVALID_ROOM_INDEX;
				SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
				return false;
			}

			// 방 퇴장 처리
			room->LeaveRoom(user->GetRoomIndex());

			// 방 퇴장 알림
			packet_res.id_ = static_cast<short>(PacketId::ROOM_LEAVE_RES);
			packet_res.total_size_ = sizeof PacketBasicRes;
			packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
		}
		else if (true == user->IsDomainLobby())
		{
			PacketBasicRes packet_res;
			auto lobby = lobby_mgr_->GetLobby(user->GetLobbyIndex());

			// 로비 퇴장 처리
			lobby->LeaveLobby(user->GetSessionIndex());

			// 로비 퇴장 알림
			packet_res.id_ = static_cast<short>(PacketId::LOBBY_LEAVE_RES);
			packet_res.total_size_ = sizeof PacketBasicRes;
			packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
		}

		auto result = user_mgr_->RemoveUser(session_index);
		if (ErrorCode::NONE == result)
		{
			return true;
		}
		return false;
	}
}