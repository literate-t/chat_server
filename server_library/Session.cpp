#include "stdafx.h"

namespace server_library
{
	using LockGuard = Lock::LockGuard;

	void Session::Init(const SOCKET listen_socket, const int index, const SessionConfig* config, ILog* log)
	{
		log_ = log;
		listen_socket_ = listen_socket;
		index_ = index;
		recv_buf_size_		= config->max_recv_buffer_size_;
		send_buf_size_		= config->max_send_buffer_size_;
		max_packet_size_	= config->max_packet_size_;

		ring_recv_buffer_.Create(recv_buf_size_);
		ring_send_buffer_.Create(send_buf_size_);
		Init();

		recv_overlapped_ex_ = new OverlappedEx(index_);
		send_overlapped_ex_ = new OverlappedEx(index_);

		connection_msg_.type_ = MessageType::CONNECTION;
		connection_msg_.contents_ = nullptr;
		close_msg_.type_ = MessageType::CLOSE;
		close_msg_.contents_ = nullptr;

		AcceptExSocket();
	}

	void Session::Init()
	{
		memset(ip_, 0, kMaxIpLength);

		ring_recv_buffer_.Init();
		ring_send_buffer_.Init();

		connected_ = FALSE;
		sendable_ = TRUE;

		accept_io_count_ = 0;
		recv_io_count_ = 0;
		send_io_count_ = 0;
	}

	void Session::SetLog(ILog* log)
	{
		log_ = log;
	}

	Result Session::AcceptExSocket()
	{
		memset(&recv_overlapped_ex_->overlapped_, 0, sizeof OVERLAPPED);

		recv_overlapped_ex_->wsabuf_.buf = addr_buf_;
		recv_overlapped_ex_->wsabuf_.len = recv_buf_size_;
		recv_overlapped_ex_->mode_ = IoMode::ACCEPT;
		recv_overlapped_ex_->session_index_ = index_;

		client_socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == client_socket_)
		{
			log_->Write(LogType::L_ERROR, "%s | WSASocket() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_WSASOCKET;
		}
		IncrementAcceptIoCount();
		DWORD bytes = 0;
		auto result = AcceptEx(
			listen_socket_,
			client_socket_,
			recv_overlapped_ex_->wsabuf_.buf,
			0,
			sizeof SOCKADDR_IN + 16,
			sizeof SOCKADDR_IN + 16,
			&bytes,
			reinterpret_cast<OVERLAPPED*>(recv_overlapped_ex_)
		);

		if (!result && WSAGetLastError() != WSA_IO_PENDING)
		{
			DecrementAcceptIoCount();
			log_->Write(LogType::L_ERROR, "%s | AcceptEx() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_ACCEPTEX;
		}
		return Result::SUCCESS;
	}

	bool Session::CloseCompletely(bool forced)
	{
		// 비정상 종료. 강제로 클라이언트를 끊어어야 하는 경우
		if (true == forced)
		{
			Disconnect(true);
			log_->Write(LogType::L_INFO, "%s | 1. forced disconn return true", __FUNCTION__);
			return true;
		}

		else if (connected_ && accept_io_count_ == 0 && recv_io_count_ == 0 && send_io_count_ == 0)
		{
			log_->Write(LogType::L_INFO, "%s | 2. normal disconn return true", __FUNCTION__);
			Disconnect();
			return true;
		}		

		// 한 번만 접속 종료 처리 하기 위함
		else if (connected_ == FALSE && accept_io_count_ == 0 && recv_io_count_ == 0 && send_io_count_ == 0)
		{
			log_->Write(LogType::L_INFO, "%s | 3. already disconn return true", __FUNCTION__);
			return true;
		}
		return false;
	}

	void Session::Disconnect(bool forced)
	{
		SetStateDisConnected();
		LockGuard lock(cs_);
		if (INVALID_SOCKET != client_socket_)
		{
			if (true == forced)
			{
				LINGER linger = { 0 };
				linger.l_onoff = 1;
				setsockopt(client_socket_, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&linger), sizeof LINGER);
			}
			closesocket(client_socket_);
		}
	}

	Result Session::ResetSession()
	{
		LockGuard lock(cs_);

		recv_overlapped_ex_->remain_ = 0;
		recv_overlapped_ex_->total_bytes_ = 0;
		send_overlapped_ex_->remain_ = 0;
		send_overlapped_ex_->total_bytes_ = 0;
		Init();
		return AcceptExSocket();
	}

	bool Session::BindIocp(const HANDLE worker_iocp)
	{
		LockGuard lock(cs_);
		auto iocp = CreateIoCompletionPort(
			reinterpret_cast<HANDLE>(client_socket_),
			worker_iocp,
			reinterpret_cast<ULONG_PTR>(this),
			0
		);
		if (INVALID_HANDLE_VALUE == iocp || iocp != worker_iocp)
		{
			log_->Write(LogType::L_ERROR, "%s | CreateIoCompletionPort() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}
		return true;
	}

	Result Session::PostRecv(const int forward_length)
	{
		assert(TRUE == connected_ && nullptr != recv_overlapped_ex_);

		recv_overlapped_ex_->mode_ = IoMode::RECV;
		//recv_overlapped_ex_->remain_ = remain;

		recv_overlapped_ex_->wsabuf_.len = max_packet_size_;
		recv_overlapped_ex_->wsabuf_.buf = ring_recv_buffer_.ForwardRecvPos(forward_length);
		assert(nullptr != recv_overlapped_ex_->wsabuf_.buf);

		memset(&recv_overlapped_ex_->overlapped_, 0, sizeof WSAOVERLAPPED);
		IncrementRecvIoCount();

		DWORD flag = 0;
		DWORD recvBytes = 0;
		auto result = WSARecv(
			client_socket_,
			&recv_overlapped_ex_->wsabuf_,
			1,
			&recvBytes,
			&flag,
			&recv_overlapped_ex_->overlapped_,
			nullptr
		);

		if (SOCKET_ERROR == result && WSAGetLastError() != WSA_IO_PENDING)
		{
			DecrementRecvIoCount();
			log_->Write(LogType::L_ERROR, "%s | WSARecv() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::POSTRECV_NULL_SOCKET_ERROR;
		}
		else
		{
			// Main::Run()에서 sleep을 걸어도 패킷을 제대로 처리 못하는 상황이 생기는데
			// 이 부분에서 log_->Write 함수를 호출하거나 sleep을 걸면 오류가 안 생긴다.
			// 멀티스레드 환경이기 때문에 순서상의 어떤 문제가 있는 것으로 생각됨
			// 아직까지는 최선의 해결책
			this_thread::sleep_for(chrono::milliseconds(1));
			//log_->Write(LogType::L_WARN, "%s | WSARecv() completed", __FUNCTION__, WSAGetLastError());
		}
		return Result::SUCCESS;
	}

	bool Session::PostSend(const int sendSize, char* buffer)
	{
		if (InterlockedCompareExchange(reinterpret_cast<long*>(&sendable_), FALSE, TRUE))
		{
			if (nullptr == buffer)
			{
				InterlockedExchange(reinterpret_cast<long*>(&sendable_), TRUE);
				log_->Write(LogType::L_ERROR, "%s | ring_send_buffer_.GetBuffer() failure", __FUNCTION__);
				return false;
			}

			memset(&send_overlapped_ex_->overlapped_, 0, sizeof WSAOVERLAPPED);

			send_overlapped_ex_->wsabuf_.buf = buffer;
			send_overlapped_ex_->wsabuf_.len = sendSize;
			send_overlapped_ex_->session_index_ = index_;

			send_overlapped_ex_->remain_ = 0;
			send_overlapped_ex_->total_bytes_ = sendSize;
			send_overlapped_ex_->mode_ = IoMode::SEND;

			IncrementSendIoCount();

			DWORD sendBytes = 0;
			auto result = WSASend(
				client_socket_,
				&send_overlapped_ex_->wsabuf_,
				1,
				&sendBytes,
				0,
				&send_overlapped_ex_->overlapped_,
				nullptr
			);

			if (SOCKET_ERROR == result && WSAGetLastError() != WSA_IO_PENDING)
			{
				DecrementSendIoCount();
				log_->Write(LogType::L_ERROR, "%s | WSASend() failure[%d]", __FUNCTION__, WSAGetLastError());
				return false;
			}
			else
			{
				// Main::Run()에서는 어디에 sleep을 걸어도 패킷을 제대로 처리 못하는 상황이 생기는데
				// 이 부분에서 log_->Write 함수를 호출하거나 sleep을 걸면 오류가 안 생긴다.
				// 멀티스레드 환경이기 때문에 순서상의 어떤 문제가 있는 것으로 생각됨
				this_thread::sleep_for(chrono::milliseconds(1));
				//log_->Write(LogType::L_WARN, "%s | WSASend() completed", __FUNCTION__);
			}
		}
		return true;
	}

	Result Session::ReserveSendPacketBuffer(OUT char** buf, const int size)
	{
		if (!connected_)
		{
			*buf = nullptr;			
			return Result::RESERVED_BUFFER_NOT_CONNECTED;
		}

		*buf = ring_send_buffer_.ForwardSendPos(size);
		if (nullptr  == *buf)
		{			
			return Result::RESERVED_BUFFER_EMPTY;
		}
		return Result::SUCCESS;
	}

	bool Session::SetAddressInfo()
	{
		SOCKADDR* localAddr		= nullptr;
		SOCKADDR* remoteAddr	= nullptr;
		int localAddrLen = 0;
		int remoteAddrLen = 0;

		GetAcceptExSockaddrs(
			addr_buf_, 0,
			sizeof SOCKADDR_IN + 16,
			sizeof SOCKADDR_IN + 16,
			&localAddr, &localAddrLen,
			&remoteAddr, &remoteAddrLen
		);

		if (0 != remoteAddrLen)
		{
			SOCKADDR_IN* remoteAddrIn = reinterpret_cast<SOCKADDR_IN*>(remoteAddr);
			if (nullptr != remoteAddrIn)
			{
				char ip[kMaxIpLength] = { 0 };
				inet_ntop(AF_INET, &remoteAddrIn->sin_addr, ip, kMaxIpLength);
				SetIp(ip);
			}
			return true;
		}
		return false;
	}
}
