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
		int MaxLobbyCount;
		int MaxLobbyUserCount;
		int MaxRoomCount;
		int MaxRoomUserCount;
	};

	struct LobbySmallInfo
	{
		short Num;
		short UserCount;
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
		ILog*		Log		= nullptr;
		IocpServer* Server	= nullptr;
		std::vector<Lobby> LobbyList;
	};
}