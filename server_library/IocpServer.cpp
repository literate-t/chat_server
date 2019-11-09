#include "stdafx.h"

namespace library
{
	IocpServer::IocpServer()
	{
		socket_listener_ = 0;
		worker_iocp_ = INVALID_HANDLE_VALUE;
		process_iocp_ = INVALID_HANDLE_VALUE;
		process_thread_ = INVALID_HANDLE_VALUE;

		port_ = -1;

		time_tick_ = 0;
		worker_thread_count_ = 0;
		process_thread_count_ = 0;

		worker_thread_flag_ = false;
		process_thread_flag_ = false;

		memset(&process_packet_, 0, sizeof PacketProcess);
		process_packet_cnt_ = 0;
	}
}