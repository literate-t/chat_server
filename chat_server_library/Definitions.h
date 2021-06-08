#pragma once
#include "../server_library/Definitions.h"

namespace chat_server_library
{
	struct ChatConfig : server_library::ServerConfig
	{
		int post_message_threads_count_ = 1;
		int start_room_number_ = 0;
		int max_room_count_ = 0;
		int max_room_user_count_ = 0;

		server_library::ServerConfig GetServerConfig()
		{
			server_library::ServerConfig config;

			config.port_ = port_;
			config.worker_thread_count_ = worker_thread_count_;
			config.session_max_recv_buffer_size_ = session_max_recv_buffer_size_;
			config.session_max_send_buffer_size_ = session_max_send_buffer_size_;
			config.max_packet_size_ = max_packet_size_;
			config.max_session_count_ = max_session_count_;
			config.max_message_pool_count_ = max_message_pool_count_;
			config.extra_message_pool_count_ = extra_message_pool_count_;
			config.performance_packet_millisec_time_ = performance_packet_millisec_time_;

			return config;
		}
	};
}