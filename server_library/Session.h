#pragma once
#include "stdafx.h"

namespace server_library
{
	class Session
	{
		using LockGuard = Lock::LockGuard;

	public:
		Session() = default;
		~Session() {}

		Message* GetConnectionMsg() { return &connection_msg_; }
		Message* GetCloseMsg() { return &close_msg_; }

		void Init(const SOCKET listen_socket, const int index, const SessionConfig* config, ILog* log);
		void SetLog(ILog* log);

		bool CloseCompletely(bool forced = false);
		void Disconnect(bool forced = false);
		Result ResetSession();
		bool BindIocp(const HANDLE WorkerIocp);
		//Result PostRecv(const int forwardLength, const DWORD remainByte);
		Result PostRecv(const int forward_length);
		bool PostSend(const int send_size, char* buffer);
		Result ReserveSendPacketBuffer(OUT char** buf, const int send_size);

		SOCKET GetClientSocket() { return client_socket_; }
		void SetIp(const char* ip) { memcpy(ip_, ip, kMaxIpLength); }
		int GetIndex() { return index_; }

		void IncrementAcceptIoCount() { ++accept_io_count_; }
		void DecrementAcceptIoCount() { accept_io_count_ = accept_io_count_ ? --accept_io_count_ : 0; }
		void IncrementRecvIoCount() { ++recv_io_count_; }
		void DecrementRecvIoCount() { recv_io_count_ = recv_io_count_ ? --recv_io_count_ : 0; }
		void IncrementSendIoCount() { ++send_io_count_; }
		void DecrementSendIoCount() { send_io_count_ = send_io_count_ ? --send_io_count_ : 0; }
		short GetRecvIoCount() { return recv_io_count_.load(); }

		bool IsConnected() { return connected_; }
		void SetStateConnected() { InterlockedExchange(reinterpret_cast<long*>(&connected_), TRUE); }
		void SetStateDisConnected() { InterlockedExchange(reinterpret_cast<long*>(&connected_), FALSE); }

		int GetRecvBufferSize() { return ring_recv_buffer_.GetBufferSize(); }
		char* GetRecvBufferBegin() { return ring_recv_buffer_.GetBegin();}
		void ReleaseRecvBuffer(const int size) { ring_recv_buffer_.ReleaseBuffer(size); }

		bool SetAddressInfo();
		void ReleaseSendBuffer(const int size) { ring_send_buffer_.ReleaseBuffer(size); }
		void SetSendAvaliable() { InterlockedExchange(reinterpret_cast<long*>(&sendable_), TRUE); }
		

	private:
		void Init();
		Result AcceptExSocket();		

	private:
		int index_ = -1;
		SOCKET client_socket_ = INVALID_SOCKET;
		SOCKET listen_socket_ = INVALID_SOCKET;

		OverlappedEx*	recv_overlapped_ex_ = nullptr;
		OverlappedEx*	send_overlapped_ex_ = nullptr;

		RingBuffer		ring_recv_buffer_;
		RingBuffer		ring_send_buffer_;
		int				max_packet_size_ = -1;

		char			addr_buf_[kMaxAddrLength] = { 0 };

		BOOL connected_	= FALSE;
		BOOL sendable_	= FALSE;

		int recv_buf_size_ = -1;
		int send_buf_size_ = -1;

		char ip_[kMaxIpLength] = { 0 };

		atomic<short> accept_io_count_	= 0;
		atomic<short> recv_io_count_	= 0;
		atomic<short> send_io_count_	= 0;

		Message connection_msg_;
		Message close_msg_;

		ILog*	log_;
		Lock	cs_;
	};
}