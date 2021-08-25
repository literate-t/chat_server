#include "stdafx.h"

namespace chat_server_library
{
	void PacketManager::RoomEnter(const int session_index, char* buf, const short copy_size)
	{
		if (sizeof PacketBasicEnterLeaveReq != copy_size)
		{
			return;
		}

		PacketId packet_id = PacketId::ROOM_ENTER_RES;

		auto [error_code, user] = user_mgr_->GetUser(session_index);
		if (ErrorCode::NONE != error_code)
		{
			PacketBasicRes packet_res = GetPacketBasicRes(packet_id, error_code);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		if (false == user->IsDomainLobby())
		{
			PacketBasicRes packet_res = 
				GetPacketBasicRes(packet_id, ErrorCode::ROOM_ENTER_INVALID_DOMAIN);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto lobby = lobby_mgr_->GetLobby(user->GetLobbyIndex());
		if (nullptr == lobby)
		{
			PacketBasicRes packet_res =
				GetPacketBasicRes(packet_id, ErrorCode::ROOM_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto data = GetPacketBasicEnterLeaveReq(buf);
		auto room = lobby->GetRoom(data->index_);
		if (nullptr == room)
		{
			room = lobby->CreateRoom(data->index_);
			if (nullptr == room)
			{
				PacketBasicRes packet_res =
					GetPacketBasicRes(packet_id, ErrorCode::ROOM_ENTER_INVALID_ROOM_INDEX);
				SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
				return;
			}

			// �� �ε��� ��ȣ�� Ÿ��Ʋ��
			char title[2] = { 0 };
			sprintf_s(title, 2, "%d", data->index_);
			room->SetRoom(lobby->GetIndex(), title);
		}

		// �� ����
		auto result = room->EnterRoom(user);
		if (ErrorCode::NONE != result)
		{
			PacketBasicRes packet_res =	GetPacketBasicRes(packet_id, result);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		// �κ� ��Ͽ��� ���ֱ� ���� �˸�
		PacketBasicRes lobby_leave_res = GetPacketBasicRes(PacketId::LOBBY_LEAVE_RES, ErrorCode::NONE);
		SendPacketFunc(session_index, &lobby_leave_res, lobby_leave_res.total_size_);
		lobby->NotifyToAll(PacketId::LOBBY_LEAVE_USER_NTF, user->GetIndex());

		// lobby���� ������ Domain�� Room���� ����
		lobby->LeaveLobbyToEnterRoom(user->GetIndex());

		// �� ���� �˸�
		room->SendAllUsersInfoToSession(PacketId::ROOM_ENTER_RES, session_index);
		room->NotifyToAll(PacketId::ROOM_ENTER_USER_NTF, user->GetIndex());
	}

	void PacketManager::RoomLeave(const int session_index, char* buf, const short copy_size)
	{
		if (sizeof PacketBasicEnterLeaveReq != copy_size)
		{
			return;
		}

		PacketId packet_id = PacketId::ROOM_LEAVE_RES;

		auto [error_code, user] = user_mgr_->GetUser(session_index);
		if (ErrorCode::NONE != error_code)
		{
			PacketBasicRes packet_res = GetPacketBasicRes(packet_id, error_code);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		if (false == user->IsDomainRoom())
		{
			PacketBasicRes packet_res = 
				GetPacketBasicRes(packet_id, ErrorCode::ROOM_ENTER_INVALID_DOMAIN);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto lobby = lobby_mgr_->GetLobby(user->GetLobbyIndex());
		if (nullptr == lobby)
		{
			PacketBasicRes packet_res = 
				GetPacketBasicRes(packet_id, ErrorCode::ROOM_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto data = GetPacketBasicEnterLeaveReq(buf);
		auto room = lobby->GetRoom(data->index_);
		if (nullptr == room)
		{
			PacketBasicRes packet_res =
				GetPacketBasicRes(packet_id, ErrorCode::ROOM_LEAVE_INVALID_ROOM_INDEX);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		// �뿡 ���� �ִ� ���ǿ��� �����ϴ� ������ �˸�
		room->NotifyToAll(PacketId::ROOM_LEAVE_USER_NTF, user->GetIndex());

		// �濡�� ���� ���� ����
		auto result = room->LeaveRoom(user->GetIndex());
		if (ErrorCode::NONE != result)
		{
			PacketBasicRes packet_res =	GetPacketBasicRes(packet_id, result);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;;
		}

		// �� ����
		PacketBasicRes room_leavae_res = GetPacketBasicRes(packet_id, ErrorCode::NONE);
		SendPacketFunc(session_index, &room_leavae_res, room_leavae_res.total_size_);

		// �κ� ����
		lobby->EnterLobby(user);
		PacketBasicRes lobby_enter_res = GetPacketBasicRes(PacketId::LOBBY_ENTER_RES, ErrorCode::NONE);
		SendPacketFunc(session_index, &lobby_enter_res, lobby_enter_res.total_size_);

		// �κ� ���� ����
		lobby->SendAllUsersInfoToSession(PacketId::LOBBY_ENTER_USER_INFO, session_index);
		lobby->NotifyToAll(PacketId::LOBBY_ENTER_USER_NTF, user->GetIndex());
	}

	void PacketManager::RoomChat(int session_index, char* buf, short copy_size)
	{
		auto [error_code, user] = user_mgr_->GetUser(session_index);

		PacketId packet_id = PacketId::ROOM_CHAT_RES;
		if (ErrorCode::NONE != error_code)
		{
			PacketBasicRes packet_res = GetPacketBasicRes(packet_id, error_code);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto lobby = lobby_mgr_->GetLobby(user->GetLobbyIndex());
		if (nullptr == lobby)
		{
			PacketBasicRes packet_res = 
				GetPacketBasicRes(packet_id, ErrorCode::LOBBY_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto room = lobby->GetRoom(user->GetRoomIndex());
		if (nullptr == room)
		{
			PacketBasicRes packet_res =
				GetPacketBasicRes(packet_id, ErrorCode::ROOM_ENTER_INVALID_ROOM_INDEX);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		// Ŭ���̾�Ʈ�� �ؽ�Ʈ�� �����µ� �������� �޽����� ������ ���� ������ ���� ����
		// ������ ����� �� �����Ⱚ�� ���ԵǹǷ� �������� �޽��� ���� �� ����
		//auto body_size = copy_size - kPacketHeaderLength;
		//buf[kPacketHeaderLength + body_size] = '\0';
		auto msg = &buf[kPacketHeaderLength];
		room->SendChat(user->GetId(), user->GetRoomIndex(), msg);
	}
}