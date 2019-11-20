#include "stdafx.h"

namespace ChatServerLibrary
{
	void PacketManager::LobbyEnter(const int sessionIndex, char* buf, short copySize)
	{
		PacketBasicRes packetRes;
		packetRes.TotalSize = sizeof PacketBasicRes;
		packetRes.Id = static_cast<unsigned short>(PacketId::LOBBY_ENTER_RES);

		auto userResult = UserMgr->GetUser(sessionIndex);
		auto errorCode = std::get<0>(userResult);
		if (errorCode != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<unsigned short>(errorCode);
			SendPacketFunc(sessionIndex, &packetRes, sizeof packetRes);
			return;
		}

		auto user = std::get<1>(userResult);
		if (user->IsDomainLogin() == false)
		{
			packetRes.ErrorCode = static_cast<unsigned short>(ErrorCode::LOBBY_ENTER_INVALID_DOMAIN);
			SendPacketFunc(sessionIndex, &packetRes, sizeof packetRes);
			return;
		}

		auto data = reinterpret_cast<PacketBasicEnterLeaveReq*>(&buf[kPacketHeaderLength]);
		auto lobby = LobbyMgr->GetLobby(data->Index);
		if (lobby == nullptr)
		{
			packetRes.ErrorCode = static_cast<unsigned short>(ErrorCode::LOBBY_ENTER_INVALID_LOBBY_INDEX);
			SendPacketFunc(sessionIndex, &packetRes, sizeof packetRes);
			return;
		}

		auto enterResult = lobby->EnterLobby(user);
		if (enterResult != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<unsigned short>(enterResult);
			SendPacketFunc(sessionIndex, &packetRes, sizeof packetRes);
			return;
		}

		// �κ� ����
		packetRes.ErrorCode = (short)ErrorCode::NONE;
		SendPacketFunc(sessionIndex, &packetRes, sizeof packetRes);

		// �� ������ ���� ���� ���� ������ �� �޴´�
		lobby->SendAllUsersInfoToSession((short)PacketId::LOBBY_ENTER_USER_INFO, sessionIndex);

		// ���� ������ �� ���� ������ �޴´�
		lobby->NotifyToAll((short)PacketId::LOBBY_ENTER_USER_NTF, user->GetIndex());
	}
}