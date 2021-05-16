#pragma once
#include "stdafx.h"

namespace ServerLibrary
{
	class IocpServer;
	class ILog;
}

namespace Common
{
	enum class ErrorCode : short;
}

namespace ChatServerLibrary
{
	struct LobbyManagerConfig
	{
		int max_lobby_count_;
		int max_lobby_user_count_;
		int max_room_count_;
		int max_room_user_count_;
	};

	struct LobbySmallInfo
	{
		short num_;
		short user_count_;
	};

	using ErrorCode = Common::ErrorCode;
	class Lobby;
	class LobbyManager
	{
		using IocpServer = ServerLibrary::IocpServer;
		using ILog		 = ServerLibrary::ILog;

	public:
		LobbyManager();
		~LobbyManager();

		void Init(LobbyManagerConfig* config, IocpServer* server, ILog* log);
		Lobby* GetLobby(short lobbyId);

		function<void(int, void*, short)> SendPacketFunc;

	private:
		ILog*		log_		= nullptr;
		IocpServer* server_	= nullptr;
		std::vector<Lobby> lobby_list_;
	};
}