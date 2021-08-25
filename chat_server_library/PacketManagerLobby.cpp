#include "stdafx.h"

namespace chat_server_library
{
	void PacketManager::LobbyEnter(const int session_index, char* buf, const short copy_size)
	{
		if (sizeof PacketBasicEnterLeaveReq != copy_size)
		{
			return;
		}

		PacketId packet_id = PacketId::LOBBY_ENTER_RES;

		auto [error_code, user] = user_mgr_->GetUser(session_index);
		if (ErrorCode::NONE != error_code)
		{
			PacketBasicRes packet_res = GetPacketBasicRes(packet_id, error_code);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
			return;
		}

		auto data = GetPacketBasicEnterLeaveReq(buf);
		auto lobby = lobby_mgr_->GetLobby(data->index_);
		if (nullptr == lobby)
		{
			PacketBasicRes packet_res = 
				GetPacketBasicRes(packet_id, ErrorCode::LOBBY_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
			return;
		}

		auto enter_result = lobby->EnterLobby(user);
		if (ErrorCode::NONE != enter_result)
		{
			PacketBasicRes packet_res = GetPacketBasicRes(packet_id, enter_result);
			SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);
			return;
		}

		// �κ� ����
		PacketBasicRes packet_res =	GetPacketBasicRes(packet_id, ErrorCode::NONE);
		SendPacketFunc(session_index, &packet_res, sizeof PacketBasicRes);

		// ���� ������ ������ ���� ���� ���� ������ �޴´�
		lobby->SendAllUsersInfoToSession(PacketId::LOBBY_ENTER_USER_INFO, session_index);

		// ���� ������ �� ������ �����޴´�
		lobby->NotifyToAll(PacketId::LOBBY_ENTER_USER_NTF, user->GetIndex());
	}

	void PacketManager::LobbyLeave(const int session_index, char* buf, const short copy_size)
	{
		if (sizeof PacketBasicEnterLeaveReq != copy_size)
		{
			return;
		}
		PacketId packet_id = PacketId::LOBBY_LEAVE_RES;

		auto [error_code, user] = user_mgr_->GetUser(session_index);
		if (ErrorCode::NONE != error_code)
		{
			PacketBasicRes packet_res = GetPacketBasicRes(packet_id, error_code);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		if (false == user->IsDomainLobby())
		{
			PacketBasicRes packet_res = GetPacketBasicRes(packet_id, ErrorCode::LOBBY_LEAVE_INVALID_DOMAIN);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto data = GetPacketBasicEnterLeaveReq(buf);
		auto lobby = lobby_mgr_->GetLobby(data->index_);
		if (nullptr == lobby)
		{
			PacketBasicRes packet_res = GetPacketBasicRes(packet_id, ErrorCode::LOBBY_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		// �κ� ������ ��û�� ����
		PacketBasicRes packet_res = GetPacketBasicRes(packet_id, ErrorCode::NONE);
		SendPacketFunc(session_index, &packet_res, packet_res.total_size_);

		// �κ� ���� �ִ� ���ǿ��� �����ϴ� ������ ����
		lobby->NotifyToAll(PacketId::LOBBY_LEAVE_USER_NTF, user->GetIndex());

		auto leave_result = lobby->LeaveLobby(user->GetIndex());
		
		if (ErrorCode::NONE != leave_result)
		{
			PacketBasicRes packet_res = GetPacketBasicRes(packet_id, leave_result);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
		}
	}
}