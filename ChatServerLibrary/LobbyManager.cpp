//#include "Packet.h"
//#include "LobbyManager.h"
//#include "Lobby.h"
#include "stdafx.h"

using PacketId = Common::PacketId;
namespace ChatServerLibrary
{
	LobbyManager::LobbyManager()  {}
	LobbyManager::~LobbyManager() {}

	void LobbyManager::Init(LobbyManagerConfig* config, IocpServer* server, ILog* log)
	{
		Log = log;
		Server = server;

		for (int i = 0; i < config->MaxLobbyCount; ++i) {
			Lobby lobby;
			lobby.SendPacketFunc = SendPacketFunc;
			lobby.Init((short)i, (short)config->MaxLobbyCount, (short)config->MaxRoomCount, (short)config->MaxRoomUserCount, Log);
			//lobby.Set(Server, Log);
			LobbyList.push_back(lobby);
		}
	}

	Lobby* LobbyManager::GetLobby(short lobbyIndex)
	{
		if (lobbyIndex < 0 || lobbyIndex >= (short)LobbyList.size() - 1) {
			return nullptr;
		}

		return &LobbyList[lobbyIndex];
	}
}