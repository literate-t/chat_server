#include "stdafx.h"

namespace ChatServerLibrary
{
	void PacketManager::Init(UserManager* user_manager, LobbyManager* lobby_manager, ServerLibrary::ILog* log)
	{
		PacketFuncDictionary[static_cast<short>(PacketId::LOGIN_REQ)] = &PacketManager::Login;
		PacketFuncDictionary[static_cast<short>(PacketId::LOBBY_ENTER_REQ)] = &PacketManager::LobbyEnter;
		PacketFuncDictionary[static_cast<short>(PacketId::LOBBY_LEAVE_REQ)] = &PacketManager::LobbyLeave;
		PacketFuncDictionary[static_cast<short>(PacketId::ROOM_ENTER_REQ)]	 = &PacketManager::RoomEnter;
		PacketFuncDictionary[static_cast<short>(PacketId::ROOM_LEAVE_REQ)] = &PacketManager::RoomLeave;
		PacketFuncDictionary[static_cast<short>(PacketId::ROOM_CHAT_REQ)] = &PacketManager::RoomChat;

		user_mgr_ = user_manager;
		lobby_mgr_ = lobby_manager;
		log_ = log;
	}

	void PacketManager::ProcessPacket(int session_index, char* buf, short copy_size)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buf);
		auto iter = PacketFuncDictionary.find(header->id_);
		if (iter != PacketFuncDictionary.end())
		{
			(this->*(iter->second))(session_index, buf, copy_size);
		}
	}

	void PacketManager::Login(const int session_index, char* buf, short copy_size)
	{
		if (copy_size - static_cast<short>(kPacketHeaderLength) != static_cast<short>(kLoginReqPacketSize))
		{
			return;
		}

		auto login_req = reinterpret_cast<PacketLoginReq*>(&buf[kPacketHeaderLength]);		
		log_->Write(ServerLibrary::LogType::L_INFO, "id_:%s", login_req->id_);
		PacketBasicRes packet_res;
		packet_res.total_size_ = sizeof PacketBasicRes;
		packet_res.id_		= static_cast<short>(PacketId::LOGIN_RES);
		auto result = user_mgr_->AddUser(session_index, login_req->id_);
		if (result != ErrorCode::NONE)
		{
			packet_res.error_code_ = static_cast<short>(result);
			SendPacketFunc(session_index, &packet_res, sizeof packet_res);
			return;
		}

		packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
	}

	bool PacketManager::ProcessLogoff(const int session_index)
	{
		auto result = user_mgr_->RemoveUser(session_index);
		if (result == ErrorCode::NONE)
		{
			return true;
		}
		return false;
	}
}