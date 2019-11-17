//#include "Packet.h"
//#include "LobbyManager.h"
//#include "Lobby.h"
#include "stdafx.h"

using PacketId = Common::PacketId;
namespace ChatServerLibrary
{
	LobbyManager::LobbyManager()  {}
	LobbyManager::~LobbyManager() {}

	void LobbyManager::Init(LobbyManagerConfig* config, IocpServer* server, ILog* logger)
	{
		Log = logger;
		Server = server;

		for (int i = 0; i < config->MaxLobbyCount; ++i) {
			Lobby lobby;
			lobby.Init((short)i, (short)config->MaxLobbyCount, (short)config->MaxRoomCount, (short)config->MaxRoomUserCount);
			lobby.Set(Server, Log);
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