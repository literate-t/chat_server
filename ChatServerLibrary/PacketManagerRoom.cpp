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
			// 방 인덱스 번호를 타이틀로
			char title[2] = { 0 };
			sprintf_s(title, 2, "%d", data->index_);
			room->SetRoom(lobby->GetIndex(), title);
		}

		// 방 입장
		auto result = room->EnterRoom(user);
		if (result != ErrorCode::NONE)
		{
			packet_res.error_code_ = static_cast<short>(result);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		// 로비 목록에서 없애기 위한 알림
		packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
		packet_res.id_ = static_cast<short>(PacketId::LOBBY_LEAVE_RES);
		SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
		lobby->NotifyToAll(static_cast<short>(PacketId::LOBBY_LEAVE_USER_NTF), user->GetIndex());

		// lobby에서 나가고 Domain은 Room으로 변경
		lobby->LeaveLobbyToEnterRomm(user->GetIndex());

		// 방 입장 알림
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

		// 룸에 남아 있는 세션에게 퇴장하는 세션을 알림
		room->NotifyToAll(static_cast<short>(PacketId::ROOM_LEAVE_USER_NTF), user->GetIndex());

		// 방 퇴장
		auto result = room->LeaveRoom(user->GetIndex());
		if (result != ErrorCode::NONE)
		{
			packet_res.error_code_ = static_cast<short>(result);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;;
		}
		packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(session_index, &packet_res, packet_res.total_size_);

		// 로비 입장
		lobby->EnterLobby(user);
		packet_res.error_code_ = static_cast<short>(ErrorCode::NONE);
		packet_res.id_ = static_cast<short>(PacketId::LOBBY_ENTER_RES);
		SendPacketFunc(session_index, &packet_res, packet_res.total_size_);

		// 로비 입장 공지
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

		// 클라이언트는 텍스트만 보내는데 서버에서 메시지를 받으면 끝에 쓰레기 값이 붙음
		// 사이즈 계산할 때 쓰레기값이 포함되므로 서버에서 메시지 끝에 널 삽입
		auto body_size = copy_size - kPacketHeaderLength;
		buf[kPacketHeaderLength + body_size] = '\0';
		auto msg = &buf[kPacketHeaderLength];
		room->SendChat(user->GetId(), user->GetRoomIndex(), msg);
	}
}