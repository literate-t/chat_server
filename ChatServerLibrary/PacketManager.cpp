#include "stdafx.h"

namespace ChatServerLibrary
{
	void PacketManager::Init(UserManager* userManager, LobbyManager* lobbyManager, ServerLibrary::ILog* log)
	{
		PacketFuncDictionary[static_cast<short>(PacketId::LOGIN_REQ)] = &PacketManager::Login;
		PacketFuncDictionary[static_cast<short>(PacketId::LOBBY_ENTER_REQ)] = &PacketManager::LobbyEnter;
		PacketFuncDictionary[static_cast<short>(PacketId::LOBBY_LEAVE_REQ)] = &PacketManager::LobbyLeave;
		PacketFuncDictionary[static_cast<short>(PacketId::ROOM_ENTER_REQ)]	 = &PacketManager::RoomEnter;
		PacketFuncDictionary[static_cast<short>(PacketId::ROOM_LEAVE_REQ)] = &PacketManager::RoomLeave;
		PacketFuncDictionary[static_cast<short>(PacketId::ROOM_CHAT_REQ)] = &PacketManager::RoomChat;

		UserMgr = userManager;
		LobbyMgr = lobbyManager;
		Log = log;
	}

	void PacketManager::ProcessPacket(int sessionIndex, char* buf, short copySize)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buf);
		auto iter = PacketFuncDictionary.find(header->Id);
		if (iter != PacketFuncDictionary.end())
		{
			(this->*(iter->second))(sessionIndex, buf, copySize);
		}
	}

	void PacketManager::Login(const int sessionIndex, char* buf, short copySize)
	{
		if (copySize - static_cast<short>(kPacketHeaderLength) != static_cast<short>(kLoginReqPacketSize))
		{
			return;
		}

		auto loginReq = reinterpret_cast<PacketLoginReq*>(&buf[kPacketHeaderLength]);		
		Log->Write(ServerLibrary::LogType::L_INFO, "Id:%s", loginReq->Id);
		PacketBasicRes packetRes;
		packetRes.TotalSize = sizeof PacketBasicRes;
		packetRes.Id		= static_cast<short>(PacketId::LOGIN_RES);
		auto result = UserMgr->AddUser(sessionIndex, loginReq->Id);
		if (result != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<short>(result);
			SendPacketFunc(sessionIndex, &packetRes, sizeof packetRes);
			return;
		}

		packetRes.ErrorCode = static_cast<short>(ErrorCode::NONE);
		SendPacketFunc(sessionIndex, &packetRes, sizeof PacketBasicRes);
	}

	bool PacketManager::ProcessLogoff(const int sessionIndex)
	{
		auto result = UserMgr->RemoveUser(sessionIndex);
		if (result == ErrorCode::NONE)
		{
			return true;
		}
		return false;
	}
}