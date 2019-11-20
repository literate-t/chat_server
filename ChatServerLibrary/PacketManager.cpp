
//#include "Packet.h"
//#include "PacketManager.h"
#include "stdafx.h"

using namespace Common;
namespace ChatServerLibrary
{
	void PacketManager::Init(UserManager* userManager, LobbyManager* lobbyManager, ServerLibrary::ILog* log)
	{
		PacketFuncDictionary[static_cast<unsigned short>(PacketId::LOGIN_REQ)] = &PacketManager::ProcessLogin;
		PacketFuncDictionary[static_cast<unsigned short>(PacketId::LOGOFF_REQ)] = &PacketManager::ProcessLogoff;



		UserMgr = userManager;
		LobbyMgr = lobbyManager;
		Log = log;
	}

	void PacketManager::ProcessPacket(int connectionIndex, char* buf, short copySize)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buf);
		auto iter = PacketFuncDictionary.find(header->Id);
		if (iter != PacketFuncDictionary.end())
		{
			(this->*(iter->second))(connectionIndex, buf, copySize);
		}
	}

	void PacketManager::ProcessLogin(const int connectionIndex, char* buf, short copySize)
	{
		if (copySize - static_cast<short>(kPacketHeaderLength) != static_cast<short>(kLoginReqPacketSize))
		{
			return;
		}

		auto loginReq = reinterpret_cast<PacketLoginReq*>(buf);
		auto userId = &loginReq->Id[kPacketHeaderLength];
		Log->Write(ServerLibrary::LogType::L_INFO, "Id:%s\n", userId);

		PacketBasicRes packetRes;
		packetRes.TotalSize = sizeof PacketBasicRes;
		packetRes.Id		= static_cast<unsigned short>(PacketId::LOGIN_RES);
		auto result = UserMgr->AddUser(connectionIndex, userId);
		if (result != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<unsigned short>(result);
			SendPacketFunc(connectionIndex, &packetRes, sizeof packetRes);
			//server_->SetSendingData(connectionIndex, (short)PacketId::LOGIN_RES, sizeof packetRes, (char*)&packetRes);
			return;
		}

		packetRes.ErrorCode = static_cast<unsigned short>(ErrorCode::NONE);
		SendPacketFunc(connectionIndex, &packetRes, sizeof PacketBasicRes);
	}

	void PacketManager::ProcessLogoff(const int connectionIndex, char* buf, short copySize)
	{
		if (copySize - static_cast<short>(kPacketHeaderLength) != static_cast<short>(kLoginReqPacketSize))
		{
			return;
		}

		PacketBasicRes packetRes;
		packetRes.TotalSize = sizeof PacketBasicRes;
		packetRes.Id = static_cast<unsigned short>(PacketId::LOGOFF_RES);
		auto result = UserMgr->RemoveUser(connectionIndex);
		if (result != ErrorCode::NONE)
		{
			packetRes.ErrorCode = static_cast<unsigned short>(result);
			SendPacketFunc(connectionIndex, &packetRes, sizeof PacketBasicRes);
			return;
		}	

		packetRes.ErrorCode = static_cast<unsigned short>(ErrorCode::NONE);
		SendPacketFunc(connectionIndex, &packetRes, sizeof packetRes);
	}
}