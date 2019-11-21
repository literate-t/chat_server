#pragma once
#include "stdafx.h"
//#include "../ServerLibrary/IocpServer.h"
//#include "UserManager.h"
//#include "PacketManager.h"
//#include "LobbyManager.h"

namespace ChatServerLibrary
{
	class Main
	{
	public:
		Main() = default;
		~Main() = default;

		int Init();
		void Run();
		void Stop();

	private:
		void LoadConfig();


	private:
		unique_ptr<ServerLibrary::IocpServer> Server;
		unique_ptr<ServerLibrary::ILog> Log;
		unique_ptr<ServerLibrary::ServerConfig> Config;

		unique_ptr<UserManager> UserMgr;
		unique_ptr<PacketManager> PacketMgr;
		unique_ptr<LobbyManager> LobbyMgr;

		bool IsRunning = false;
	};
}