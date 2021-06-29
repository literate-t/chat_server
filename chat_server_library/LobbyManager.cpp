#include "stdafx.h"

namespace chat_server_library
{
	LobbyManager::~LobbyManager() {}

	void LobbyManager::Init(LobbyManagerConfig* config, IocpServer* server, ILog* log)
	{
		log_ = log;
		server_ = server;

		for (int i = 0; i < config->max_lobby_count_; ++i) {
			Lobby lobby;
			lobby.SendPacketFunc = SendPacketFunc;
			lobby.Init((short)i, (short)config->max_lobby_count_, (short)config->max_room_count_, (short)config->max_room_user_count_, log_);
			lobby_list_.push_back(lobby);
		}
	}

	Lobby* LobbyManager::GetLobby(short lobby_index)
	{
		if (lobby_index < 0 || lobby_index >= (short)lobby_list_.size() - 1) {
			return nullptr;
		}
		return &lobby_list_[lobby_index];
	}
}