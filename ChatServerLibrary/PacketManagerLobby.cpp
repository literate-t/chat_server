#include "stdafx.h"

namespace ChatServerLibrary
{
	void PacketManager::LobbyEnter(const int sessionIndex, char* buf, short copySize)
	{
		if (copySize != sizeof PacketBasicEnterLeaveReq)
		{
			return;
		}

		PacketBasicRes packetRes;
		packetRes.TotalSize = sizeof PacketBasicRes;
		packetRes.Id = static_cast<short>(PacketId::LOBBY_ENTER_RES);

		auto userResult = UserMgr->GetUser(sessionIndex);
		auto errorCode = get<0>(userResult);
		if (errorCode != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(errorCode);
			SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);
			return;
		}

		auto user = get<1>(userResult);
		if (user->IsDomainLogin() == false)
		{
			packetRes.ErrorCode = static_cast<short>(ErrorCode::LOBBY_ENTER_INVALID_DOMAIN);
			SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);
			return;
		}

		auto data = reinterpret_cast<PacketBasicEnterLeaveReq*>(buf);
		auto lobby = LobbyMgr->GetLobby(data->Index);
		if (lobby == nullptr)
		{
			packetRes.ErrorCode = static_cast<short>(ErrorCode::LOBBY_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);
			return;
		}

		auto enterResult = lobby->EnterLobby(user);
		if (enterResult != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(enterResult);
			SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);
			return;
		}

		// �κ� ����
		packetRes.ErrorCode = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);

		// �� ������ ���� ���� ���� ������ �� �޴´�
		lobby->SendAllUsersInfoToSession(static_cast<short>(PacketId::LOBBY_ENTER_USER_INFO), sessionIndex);

		// ���� ������ �� ���� ������ �޴´�
		lobby->NotifyToAll(static_cast<short>(PacketId::LOBBY_ENTER_USER_NTF), user->GetIndex());
	}

	void PacketManager::LobbyLeave(int sessionIndex, char* buf, short copySize)
	{
		if (copySize != sizeof PacketBasicEnterLeaveReq)
		{
			return;
		}

		PacketBasicRes packetRes;
		packetRes.Id = static_cast<short>(PacketId::LOBBY_LEAVE_RES);
		packetRes.TotalSize = sizeof PacketBasicRes;

		//auto packet = (common::PacketBasicEnterLeaveReq*)packet_info.data_;
		auto userResult = UserMgr->GetUser(sessionIndex);
		auto errorCode = get<0>(userResult);
		if (errorCode != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(errorCode);
			SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);
			return;
		}

		auto user = get<1>(userResult);
		if (user->IsDomainLobby() == false)
		{
			packetRes.ErrorCode = static_cast<short>(ErrorCode::LOBBY_LEAVE_INVALID_DOMAIN);
			SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);
			return;
		}

		auto data = reinterpret_cast<PacketBasicEnterLeaveReq*>(buf);
		auto lobby = LobbyMgr->GetLobby(data->Index);
		if (lobby == nullptr)
		{
			packetRes.ErrorCode = static_cast<short>(ErrorCode::LOBBY_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);
			return;
		}

		// �κ� ������ ��û�� ����
		packetRes.ErrorCode = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);

		// �κ� ���� �ִ� ���ǿ��� �����ϴ� ������ ����
		lobby->NotifyToAll(static_cast<short>(PacketId::LOBBY_LEAVE_USER_NTF), user->GetIndex());

		auto leaveResult = lobby->LeaveLobby(user->GetIndex());
		if (leaveResult != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(leaveResult);
			SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);
		}
	}
}