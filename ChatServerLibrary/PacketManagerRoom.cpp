#include "stdafx.h"

namespace ChatServerLibrary
{
	void PacketManager::RoomEnter(const int sessionIndex, char* buf, short copySize)
	{
		if (copySize != sizeof PacketBasicEnterLeaveReq)
		{
			return;
		}

		PacketBasicRes packetRes;
		packetRes.Id = static_cast<short>(PacketId::ROOM_ENTER_RES);
		packetRes.TotalSize = sizeof PacketBasicRes;

		auto userResult = UserMgr->GetUser(sessionIndex);
		auto errorCode = get<0>(userResult);
		if (errorCode != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(errorCode);
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		auto user = get<1>(userResult);
		if (user->IsDomainLobby() == false)
		{
			packetRes.ErrorCode = static_cast<short>(ErrorCode::ROOM_ENTER_INVALID_DOMAIN);
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		auto lobby = LobbyMgr->GetLobby(user->GetLobbyIndex());
		if (lobby == nullptr)
		{
			packetRes.ErrorCode = (short)ErrorCode::ROOM_ENTER_INVALID_LOBBY_INDEX;
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		auto data = reinterpret_cast<PacketBasicEnterLeaveReq*>(buf);
		auto room = lobby->GetRoom(data->Index);
		if (room == nullptr)
		{
			room = lobby->CreateRoom(data->Index);
			if (room == nullptr)
			{
				packetRes.ErrorCode = (short)ErrorCode::ROOM_ENTER_INVALID_ROOM_INDEX;
				SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
				return;
			}
			// 방 인덱스 번호를 타이틀로
			char title[2] = { 0 };
			sprintf_s(title, 2, "%d", data->Index);
			room->SetRoom(lobby->GetIndex(), title);
		}

		// 방 입장
		auto result = room->EnterRoom(user);
		if (result != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(result);
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		// 로비 목록에서 없애기 위한 알림
		packetRes.ErrorCode = static_cast<short>(ErrorCode::NONE);
		packetRes.Id = static_cast<short>(PacketId::LOBBY_LEAVE_RES);
		SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
		lobby->NotifyToAll(static_cast<short>(PacketId::LOBBY_LEAVE_USER_NTF), user->GetIndex());

		// lobby에서 나가고 Domain은 Room으로 변경
		lobby->LeaveLobbyToEnterRomm(user->GetIndex());

		// 방 입장 알림
		packetRes.Id = static_cast<short>(PacketId::ROOM_ENTER_RES);
		room->SendAllUsersInfoToSession(packetRes.Id, sessionIndex);
		room->NotifyToAll(static_cast<short>(PacketId::ROOM_ENTER_USER_NTF), user->GetIndex());
	}

	void PacketManager::RoomLeave(int sessionIndex, char* buf, short copySize)
	{
		if (copySize != sizeof PacketBasicEnterLeaveReq)
		{
			return;
		}

		PacketBasicRes packetRes;
		packetRes.Id = static_cast<short>(PacketId::ROOM_LEAVE_RES);
		packetRes.TotalSize = sizeof PacketBasicRes;

		auto userResult = UserMgr->GetUser(sessionIndex);
		auto errorCode = get<0>(userResult);
		if (errorCode != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(errorCode);
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		auto user = get<1>(userResult);
		if (user->IsDomainRoom() == false)
		{
			packetRes.ErrorCode = static_cast<short>(ErrorCode::ROOM_ENTER_INVALID_DOMAIN);
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		auto lobby = LobbyMgr->GetLobby(user->GetLobbyIndex());
		if (lobby == nullptr)
		{
			packetRes.ErrorCode = (short)ErrorCode::ROOM_ENTER_INVALID_LOBBY_INDEX;
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		auto data = reinterpret_cast<PacketBasicEnterLeaveReq*>(buf);
		auto room = lobby->GetRoom(data->Index);
		if (room == nullptr)
		{
			packetRes.ErrorCode = (short)ErrorCode::ROOM_LEAVE_INVALID_ROOM_INDEX;
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		// 룸에 남아 있는 세션에게 퇴장하는 세션을 알림
		room->NotifyToAll(static_cast<short>(PacketId::ROOM_LEAVE_USER_NTF), user->GetIndex());

		// 방 퇴장
		auto result = room->LeaveRoom(user->GetIndex());
		if (result != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(result);
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;;
		}
		packetRes.ErrorCode = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);

		// 로비 입장
		lobby->EnterLobby(user);
		packetRes.ErrorCode = static_cast<short>(ErrorCode::NONE);
		packetRes.Id = static_cast<short>(PacketId::LOBBY_ENTER_RES);
		SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);

		// 로비 입장 공지
		lobby->SendAllUsersInfoToSession(static_cast<short>(PacketId::LOBBY_ENTER_USER_INFO), sessionIndex);
		lobby->NotifyToAll((short)PacketId::LOBBY_ENTER_USER_NTF, user->GetIndex());
	}

	void PacketManager::RoomChat(int sessionIndex, char* buf, short copySize)
	{
		auto userResult = UserMgr->GetUser(sessionIndex);
		auto errorCode = get<0>(userResult);

		PacketBasicRes packetRes;
		packetRes.Id = static_cast<short>(PacketId::ROOM_CHAT_RES);
		packetRes.TotalSize = sizeof PacketBasicRes;
		if (errorCode != ErrorCode::NONE)
		{
			packetRes.ErrorCode = (short)errorCode;
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		auto user = get<1>(userResult);
		auto lobby = LobbyMgr->GetLobby(user->GetLobbyIndex());
		if (lobby == nullptr)
		{
			packetRes.ErrorCode = static_cast<short>(ErrorCode::LOBBY_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		auto room = lobby->GetRoom(user->GetRoomIndex());
		if (room == nullptr)
		{
			packetRes.ErrorCode = static_cast<short>(ErrorCode::ROOM_ENTER_INVALID_ROOM_INDEX);
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		// 클라이언트는 텍스트만 보내는데 서버에서 메시지를 받으면 끝에 쓰레기 값이 붙음
		// 사이즈 계산할 때 쓰레기값이 포함되므로 서버에서 메시지 끝에 널 삽입
		auto bodySize = copySize - kPacketHeaderLength;
		buf[kPacketHeaderLength + bodySize] = '\0';
		auto msg = &buf[kPacketHeaderLength];
		room->SendChat(user->GetId(), user->GetRoomIndex(), msg);
	}
}