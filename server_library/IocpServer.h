#pragma once
#include "stdafx.h"

namespace server_library
{
	const int kPacketHeaderLength = 4;
	const int kPacketSizeLength = 2;
	const int kPacketTypeLength = 2;

	class ILog;
	class IocpServer
	{
	public:
		IocpServer() = default;
		~IocpServer() = default;

	public:
		bool Start();
		void End();
		void Init(ServerConfig* config, ILog* log);
		bool ProcessMessageIOCP(OUT char& msgType, OUT int& session_index, OUT char** buf, OUT short& copy_size, int wait_mill_sec);
		void SendPacket(const int session_index, const void* packet, const short packet_size);

		int GetMaxPacketSize()		{ return server_config_->max_packet_size_; }
		int GetMaxSessionCount() { return server_config_->max_session_count_; }

	private:
		Result CreateListenSocket();
		Result CreateIocp();
		bool CreateMessagePool();
		bool BindListenSocketIocp();
		bool CreateSessions();
		void DestroySessions();
		Session* GetSession(const int session_index);
		bool CreateWorkerThread();
		void WorkerThread();
		Result PostMessageIOCP(Session* session, Message* msg, const DWORD packet_size = 0);
		void HandleWorkerThreadException(Session* session, const OverlappedEx* overlapped_ex);
		void HandleSessionCloseException(Session* session);

		void DoAccept(const OverlappedEx* overlapped_ex);
		void DoRecv(OverlappedEx* overlapped_ex, const DWORD size);
		void DoSend(OverlappedEx* overlapped_ex, const DWORD size);
		void ForwardPacket(Session* session, DWORD& remain, char* buffer);

		void DoPostConnection(Session* session, const Message* msg, OUT char& msgType, OUT int& session_index);
		void DoPostClose(Session* session, const Message* msg, OUT char& msgType, OUT int& session_index);
		void DoPostRecvPacket(Session* session, const Message* msg, OUT char& msgType, OUT int& session_index, OUT char** buf, OUT short& copy_size, const DWORD ioSize);

	private:
		ServerConfig* server_config_ = nullptr;
		ILog* log_ = nullptr;

		SOCKET listen_socket_= INVALID_SOCKET;

		vector<Session*> session_vec_;
		HANDLE worker_iocp_= INVALID_HANDLE_VALUE;
		HANDLE message_iocp_ = INVALID_HANDLE_VALUE;

		bool is_worker_thread_running_= true;
		vector<unique_ptr<thread>> worker_thread_vec_;
		unique_ptr<MessagePool>	   unique_message_pool_;
	};
}