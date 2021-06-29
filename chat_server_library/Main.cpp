#include "stdafx.h"
#include "../server_library/Logger.h"

namespace chat_server_library
{
	int Main::Init()
	{
		config_ = make_unique<server_library::ServerConfig>();
		LoadConfig();

		log_ = make_unique<server_library::Logger>();

		server_ = make_unique<server_library::IocpServer>();
		server_->Init(config_.get(), log_.get());
		auto result = server_->Start();
		if (false == result)
		{
			log_->Write(server_library::LogType::L_ERROR, "%s | Starting server is failed", __FUNCTION__);
			return -1;
		}

		auto SendPacketFunc = [&](const int connection_index, const void* packet, const short packet_size)
		{
			server_->SendPacket(connection_index, packet, packet_size);
		};

		user_mgr_ = make_unique<UserManager>();
		user_mgr_->Init(config_->max_session_count_);
		user_mgr_->SendPacketFunc = SendPacketFunc;

		lobby_mgr_ = make_unique<LobbyManager>();
		LobbyManagerConfig config = { config_->max_lobby_count_, config_->max_lobby_user_count_,
									  config_->max_room_count_, config_->max_room_user_count_ };
		lobby_mgr_->SendPacketFunc = SendPacketFunc;
		lobby_mgr_->Init(&config, server_.get(), log_.get());

		packet_mgr_ = make_unique<PacketManager>();
		packet_mgr_->Init(user_mgr_.get(), lobby_mgr_.get(), log_.get());
		packet_mgr_->SendPacketFunc = SendPacketFunc;

		return 0;
	}

	void Main::Run()
	{
		is_running_ = true;
		auto buf = new char[config_->max_packet_size_];
		memset(buf, 0, config_->max_packet_size_);
		int wait_milli_sec= 1;

		while (is_running_)
		{
			char type = 0;
			int session_index = 0;
			short copy_size = 0;

			if (!server_->ProcessMessageIOCP(type, session_index, &buf, copy_size, wait_milli_sec))
			{
				continue;
			}

			auto msg_type = static_cast<server_library::MessageType>(type);
			switch (msg_type)
			{
				case server_library::MessageType::CONNECTION:
				{
					log_->Write(server_library::LogType::L_INFO, "On connect index:%d", session_index);
					break;
				}

				case server_library::MessageType::CLOSE:
				{
					auto result = packet_mgr_->ProcessLogoff(session_index);
					if (result)
					{
						log_->Write(server_library::LogType::L_INFO, "Session index:%d is closed", session_index);
					}
					break;
				}

				case server_library::MessageType::ONRECV:
				{
					packet_mgr_->ProcessPacket(session_index, buf, copy_size);
					break;
				}
			}
		}
		delete[] buf;
	}

	void Main::Stop()
	{
		is_running_ = false;
		server_->End();
	}

	void Main::LoadConfig()
	{
		wchar_t path[MAX_PATH] = { 0 };
		GetCurrentDirectory(MAX_PATH, path);

		wchar_t configPath[MAX_PATH] = { 0 };
		_snwprintf_s(configPath, MAX_PATH, _TRUNCATE, L"%s\\server_config.ini", path);

		config_->port_ = (unsigned short)GetPrivateProfileInt(L"server_config", L"port_", 0, configPath);
		config_->back_log_count_ = GetPrivateProfileInt(L"server_config", L"back_log_count", 0, configPath);
		config_->worker_thread_count_ = GetPrivateProfileInt(L"server_config", L"worker_thread_count", 0, configPath);
		config_->session_max_recv_buffer_size_ =	GetPrivateProfileInt(L"server_config", L"session_max_recv_buffer_size", 0, configPath);
		config_->session_max_send_buffer_size_ =	GetPrivateProfileInt(L"server_config", L"session_max_send_buffer_size", 0, configPath);
		config_->max_packet_size_ =	GetPrivateProfileInt(L"server_config", L"max_packet_size", 0, configPath);
		config_->max_session_count_ =	GetPrivateProfileInt(L"server_config", L"max_session_count", 0, configPath);
		config_->max_message_pool_count_ =	GetPrivateProfileInt(L"server_config", L"max_message_pool_count", 0, configPath);
		config_->extra_message_pool_count_ =	GetPrivateProfileInt(L"server_config", L"extra_message_pool_count", 0, configPath);
		config_->performance_packet_millisec_time_ = GetPrivateProfileInt(L"server_config", L"performance_packet_millisec_time", 0, configPath);

		config_->max_lobby_count_ = GetPrivateProfileInt(L"server_config", L"max_lobby_count", 0, configPath);
		config_->max_lobby_user_count_ = GetPrivateProfileInt(L"server_config", L"max_lobby_user_count", 0, configPath);
		config_->max_room_count_ = GetPrivateProfileInt(L"server_config", L"max_room_count", 0, configPath);
		config_->max_room_user_count_ = GetPrivateProfileInt(L"server_config", L"max_room_user_count", 0, configPath);
	}
}
