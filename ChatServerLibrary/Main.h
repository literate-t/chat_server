#pragma once
#include "stdafx.h"

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
		unique_ptr<ServerLibrary::IocpServer> server_;
		unique_ptr<ServerLibrary::ILog> log_;
		unique_ptr<ServerLibrary::ServerConfig> config_;

		unique_ptr<UserManager> user_mgr_;
		unique_ptr<PacketManager> packet_mgr_;
		unique_ptr<LobbyManager> lobby_mgr_;

		bool is_running_ = false;
	};
}