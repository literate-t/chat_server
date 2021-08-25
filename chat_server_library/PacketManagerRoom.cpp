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

			// 방 인덱스 번호를 타이틀로
			char title[2] = { 0 };
			sprintf_s(title, 2, "%d", data->index_);
			room->SetRoom(lobby->GetIndex(), title);
		}

		// 방 입장
		auto result = room->EnterRoom(user);
		if (ErrorCode::NONE != result)
		{
			PacketBasicRes packet_res =	GetPacketBasicRes(packet_id, result);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;
		}

		// 로비 목록에서 없애기 위한 알림
		PacketBasicRes lobby_leave_res = GetPacketBasicRes(PacketId::LOBBY_LEAVE_RES, ErrorCode::NONE);
		SendPacketFunc(session_index, &lobby_leave_res, lobby_leave_res.total_size_);
		lobby->NotifyToAll(PacketId::LOBBY_LEAVE_USER_NTF, user->GetIndex());

		// lobby에서 나가고 Domain은 Room으로 변경
		lobby->LeaveLobbyToEnterRoom(user->GetIndex());

		// 방 입장 알림
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

		// 룸에 남아 있는 세션에게 퇴장하는 세션을 알림
		room->NotifyToAll(PacketId::ROOM_LEAVE_USER_NTF, user->GetIndex());

		// 방에서 유저 정보 삭제
		auto result = room->LeaveRoom(user->GetIndex());
		if (ErrorCode::NONE != result)
		{
			PacketBasicRes packet_res =	GetPacketBasicRes(packet_id, result);
			SendPacketFunc(session_index, &packet_res, packet_res.total_size_);
			return;;
		}

		// 방 퇴장
		PacketBasicRes room_leavae_res = GetPacketBasicRes(packet_id, ErrorCode::NONE);
		SendPacketFunc(session_index, &room_leavae_res, room_leavae_res.total_size_);

		// 로비 입장
		lobby->EnterLobby(user);
		PacketBasicRes lobby_enter_res = GetPacketBasicRes(PacketId::LOBBY_ENTER_RES, ErrorCode::NONE);
		SendPacketFunc(session_index, &lobby_enter_res, lobby_enter_res.total_size_);

		// 로비 입장 공지
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

		// 클라이언트는 텍스트만 보내는데 서버에서 메시지를 받으면 끝에 쓰레기 값이 붙음
		// 사이즈 계산할 때 쓰레기값이 포함되므로 서버에서 메시지 끝에 널 삽입
		//auto body_size = copy_size - kPacketHeaderLength;
		//buf[kPacketHeaderLength + body_size] = '\0';
		auto msg = &buf[kPacketHeaderLength];
		room->SendChat(user->GetId(), user->GetRoomIndex(), msg);
	}
}