
//#include "Packet.h"
//#include "PacketManager.h"
#include "stdafx.h"

using namespace Common;
namespace ChatServerLibrary
{
	void PacketManager::Init(UserManager* userManager, LobbyManager* lobbyManager, ServerLibrary::ILog* log)
	{
		PacketFuncDictionary[(short)PacketId::LOGIN_REQ] = &PacketManager::ProcessLogin;


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
		if (copySize != kLoginReqPacketSize)
		{
			return;
		}

		auto loginReq = reinterpret_cast<PacketLoginReq*>(buf);
		auto userId = loginReq->Id;
		Log->Write(ServerLibrary::LogType::L_INFO, "Id:%s\n", userId);
	}
}