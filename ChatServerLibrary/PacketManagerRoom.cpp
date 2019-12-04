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
			// �� �ε��� ��ȣ�� Ÿ��Ʋ��
			char title[2] = { 0 };
			sprintf_s(title, 2, "%d", data->Index);
			room->SetRoom(lobby->GetIndex(), title);
		}

		// �� ����
		auto result = room->EnterRoom(user);
		if (result != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(result);
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;
		}

		// �κ� ��Ͽ��� ���ֱ� ���� �˸�
		packetRes.ErrorCode = static_cast<short>(ErrorCode::NONE);
		packetRes.Id = static_cast<short>(PacketId::LOBBY_LEAVE_RES);
		SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
		lobby->NotifyToAll(static_cast<short>(PacketId::LOBBY_LEAVE_USER_NTF), user->GetIndex());

		// lobby���� ������ Domain�� Room���� ����
		lobby->LeaveLobbyToEnterRomm(user->GetIndex());

		// �� ���� �˸�
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

		// �뿡 ���� �ִ� ���ǿ��� �����ϴ� ������ �˸�
		room->NotifyToAll(static_cast<short>(PacketId::ROOM_LEAVE_USER_NTF), user->GetIndex());

		// �� ����
		auto result = room->LeaveRoom(user->GetIndex());
		if (result != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(result);
			SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);
			return;;
		}
		packetRes.ErrorCode = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);

		// �κ� ����
		lobby->EnterLobby(user);
		packetRes.ErrorCode = static_cast<short>(ErrorCode::NONE);
		packetRes.Id = static_cast<short>(PacketId::LOBBY_ENTER_RES);
		SendPacketFunc(sessionIndex, &packetRes, packetRes.TotalSize);

		// �κ� ���� ����
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

		// Ŭ���̾�Ʈ�� �ؽ�Ʈ�� �����µ� �������� �޽����� ������ ���� ������ ���� ����
		// ������ ����� �� �����Ⱚ�� ���ԵǹǷ� �������� �޽��� ���� �� ����
		auto bodySize = copySize - kPacketHeaderLength;
		buf[kPacketHeaderLength + bodySize] = '\0';
		auto msg = &buf[kPacketHeaderLength];
		room->SendChat(user->GetId(), user->GetRoomIndex(), msg);
	}
}