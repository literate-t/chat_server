#include "stdafx.h"

namespace ChatServerLibrary
{
	void PacketManager::RoomEnter(const int session_index, char* buf, short copy_size)
	{
		if (copy_size != sizeof PacketBasicEnterLeaveReq)
		{
			return;
		}

		PacketBasicRes packet_res;
		packet_res.id_ = static_cast<short>(PacketId::ROOM_ENTER_RES);
		packet_res.total_size_ = sizeof PacketBasicRes;

		auto user_result = user_mgr_->GetUser(session_index);
		auto error_code = get<0>(user_result);
		if (error_code != ErrorCode::NONE)
		{
			packet_res.error_code_ = static_cast<short>(error_code);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto user = get<1>(user_result);
		if (user->IsDomainLobby() == false)
		{
			packet_res.error_code_ = static_cast<short>(ErrorCode::ROOM_ENTER_INVALID_DOMAIN);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto lobby = lobby_mgr_->GetLobby(user->GetLobbyIndex());
		if (lobby == nullptr)
		{
			packet_res.error_code_ = (short)ErrorCode::ROOM_ENTER_INVALID_LOBBY_INDEX;
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto data = reinterpret_cast<PacketBasicEnterLeaveReq*>(buf);
		auto room = lobby->GetRoom(data->index_);
		if (room == nullptr)
		{
			room = lobby->CreateRoom(data->index_);
			if (room == nullptr)
			{
				packet_res.error_code_ = (short)ErrorCode::ROOM_ENTER_INVALID_ROOM_INDEX;
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
		if (result != ErrorCode::NONE)
		{
			packet_res.error_code_ = static_cast<short>(result);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		// �κ� ��Ͽ��� ���ֱ� ���� �˸�
		packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
		packet_res.id_ = static_cast<short>(PacketId::LOBBY_LEAVE_RES);
		SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
		lobby->NotifyToAll(static_cast<short>(PacketId::LOBBY_LEAVE_USER_NTF), user->GetIndex());

		// lobby���� ������ Domain�� Room���� ����
		lobby->LeaveLobbyToEnterRomm(user->GetIndex());

		// �� ���� �˸�
		packet_res.id_ = static_cast<short>(PacketId::ROOM_ENTER_RES);
		room->SendAllUsersInfoToSession(packet_res.id_, session_index);
		room->NotifyToAll(static_cast<short>(PacketId::ROOM_ENTER_USER_NTF), user->GetIndex());
	}

	void PacketManager::RoomLeave(int session_index, char* buf, short copy_size)
	{
		if (copy_size != sizeof PacketBasicEnterLeaveReq)
		{
			return;
		}

		PacketBasicRes packet_res;
		packet_res.id_ = static_cast<short>(PacketId::ROOM_LEAVE_RES);
		packet_res.total_size_ = sizeof PacketBasicRes;

		auto user_result = user_mgr_->GetUser(session_index);
		auto error_code = get<0>(user_result);
		if (error_code != ErrorCode::NONE)
		{
			packet_res.error_code_ = static_cast<short>(error_code);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto user = get<1>(user_result);
		if (user->IsDomainRoom() == false)
		{
			packet_res.error_code_ = static_cast<short>(ErrorCode::ROOM_ENTER_INVALID_DOMAIN);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto lobby = lobby_mgr_->GetLobby(user->GetLobbyIndex());
		if (lobby == nullptr)
		{
			packet_res.error_code_ = (short)ErrorCode::ROOM_ENTER_INVALID_LOBBY_INDEX;
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto data = reinterpret_cast<PacketBasicEnterLeaveReq*>(buf);
		auto room = lobby->GetRoom(data->index_);
		if (room == nullptr)
		{
			packet_res.error_code_ = (short)ErrorCode::ROOM_LEAVE_INVALID_ROOM_INDEX;
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		// �뿡 ���� �ִ� ���ǿ��� �����ϴ� ������ �˸�
		room->NotifyToAll(static_cast<short>(PacketId::ROOM_LEAVE_USER_NTF), user->GetIndex());

		// �� ����
		auto result = room->LeaveRoom(user->GetIndex());
		if (result != ErrorCode::NONE)
		{
			packet_res.error_code_ = static_cast<short>(result);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;;
		}
		packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(session_index, &packet_res, packet_res.total_size_);

		// �κ� ����
		lobby->EnterLobby(user);
		packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
		packet_res.id_ = static_cast<short>(PacketId::LOBBY_ENTER_RES);
		SendPacketFunc(session_index, &packet_res, packet_res.total_size_);

		// �κ� ���� ����
		lobby->SendAllUsersInfoToSession(static_cast<short>(PacketId::LOBBY_ENTER_USER_INFO), session_index);
		lobby->NotifyToAll((short)PacketId::LOBBY_ENTER_USER_NTF, user->GetIndex());
	}

	void PacketManager::RoomChat(int session_index, char* buf, short copy_size)
	{
		auto user_result = user_mgr_->GetUser(session_index);
		auto error_code = get<0>(user_result);

		PacketBasicRes packet_res;
		packet_res.id_ = static_cast<short>(PacketId::ROOM_CHAT_RES);
		packet_res.total_size_ = sizeof PacketBasicRes;
		if (error_code != ErrorCode::NONE)
		{
			packet_res.error_code_ = (short)error_code;
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto user = get<1>(user_result);
		auto lobby = lobby_mgr_->GetLobby(user->GetLobbyIndex());
		if (lobby == nullptr)
		{
			packet_res.error_code_ = static_cast<short>(ErrorCode::LOBBY_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		auto room = lobby->GetRoom(user->GetRoomIndex());
		if (room == nullptr)
		{
			packet_res.error_code_ = static_cast<short>(ErrorCode::ROOM_ENTER_INVALID_ROOM_INDEX);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		// Ŭ���̾�Ʈ�� �ؽ�Ʈ�� �����µ� �������� �޽����� ������ ���� ������ ���� ����
		// ������ ����� �� �����Ⱚ�� ���ԵǹǷ� �������� �޽��� ���� �� ����
		auto body_size = copy_size - kPacketHeaderLength;
		buf[kPacketHeaderLength + body_size] = '\0';
		auto msg = &buf[kPacketHeaderLength];
		room->SendChat(user->GetId(), user->GetRoomIndex(), msg);
	}
}