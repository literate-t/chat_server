#pragma once
#include "stdafx.h"

namespace server_library
{
	class IocpServer;
	class ILog;
}

namespace Common
{
	enum class ErrorCode : short;
}

namespace chat_server_library
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
		using IocpServer = server_library::IocpServer;
		using ILog		 = server_library::ILog;

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