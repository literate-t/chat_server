#include "stdafx.h"

namespace chat_server_library
{
	void PacketManager::LobbyEnter(const int session_index, char* buf, short copy_size)
	{
		if (copy_size != sizeof PacketBasicEnterLeaveReq)
		{
			return;
		}

		PacketBasicRes packet_res;
		packet_res.total_size_ = sizeof PacketBasicRes;
		packet_res.id_ = static_cast<short>(PacketId::LOBBY_ENTER_RES);

		auto [error_code, user] = user_mgr_->GetUser(session_index);
		//auto error_code = get<0>(user_result);
		if (ErrorCode::NONE != error_code)
		{
			packet_res.error_code_ = static_cast<short>(error_code);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
			return;
		}

		//auto user = get<1>(user_result);
		if (false == user->IsDomainLogin())
		{
			packet_res.error_code_ = static_cast<short>(ErrorCode::LOBBY_ENTER_INVALID_DOMAIN);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
			return;
		}

		auto data = reinterpret_cast<PacketBasicEnterLeaveReq*>(buf);
		auto lobby = lobby_mgr_->GetLobby(data->index_);
		if (nullptr == lobby)
		{
			packet_res.error_code_ = static_cast<short>(ErrorCode::LOBBY_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
			return;
		}

		auto enter_result = lobby->EnterLobby(user);
		if (enter_result != ErrorCode::NONE)
		{
			packet_res.error_code_ = static_cast<short>(enter_result);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
			return;
		}

		// 로비 입장
		packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);

		// 새로 입장한 유저는 기존 접속 유저 정보를 받는다
		lobby->SendAllUsersInfoToSession(static_cast<short>(PacketId::LOBBY_ENTER_USER_INFO), session_index);

		// 기존 유저는 새 유저를 공지받는다
		lobby->NotifyToAll(static_cast<short>(PacketId::LOBBY_ENTER_USER_NTF), user->GetIndex());
	}

	void PacketManager::LobbyLeave(int session_index, char* buf, short copy_size)
	{
		if (copy_size != sizeof PacketBasicEnterLeaveReq)
		{
			return;
		}

		PacketBasicRes packet_res;
		packet_res.id_ = static_cast<short>(PacketId::LOBBY_LEAVE_RES);
		packet_res.total_size_ = sizeof PacketBasicRes;

		auto user_result = user_mgr_->GetUser(session_index);
		auto error_code = get<0>(user_result);
		if (error_code != ErrorCode::NONE)
		{
			packet_res.error_code_ = static_cast<short>(error_code);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
			return;
		}

		auto user = get<1>(user_result);
		if (user->IsDomainLobby() == false)
		{
			packet_res.error_code_ = static_cast<short>(ErrorCode::LOBBY_LEAVE_INVALID_DOMAIN);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
			return;
		}

		auto data = reinterpret_cast<PacketBasicEnterLeaveReq*>(buf);
		auto lobby = lobby_mgr_->GetLobby(data->index_);
		if (lobby == nullptr)
		{
			packet_res.error_code_ = static_cast<short>(ErrorCode::LOBBY_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
			return;
		}

		// 로비 퇴장을 요청한 세션
		packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);

		// 로비에 남아 있는 세션에게 퇴장하는 세션을 공지
		lobby->NotifyToAll(static_cast<short>(PacketId::LOBBY_LEAVE_USER_NTF), user->GetIndex());

		auto leave_result = lobby->LeaveLobby(user->GetIndex());
		if (leave_result != ErrorCode::NONE)
		{
			packet_res.error_code_ = static_cast<short>(leave_result);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
		}
	}
}