#include "stdafx.h"

namespace library
{
	Connection::Connection()
	{
		socket_listener_	= INVALID_SOCKET;
		recv_buf_size_		= 0;
		send_buf_size_		= 0;
		Init();
	}

	void Connection::Init()
	{
		memset(ip_, 0, MAX_IP_LENGTH);
		socket_			= INVALID_SOCKET;
		is_closed_		= false;
		is_connected_	= false;
		is_sent_		= true;
		recv_io_cnt_	= 0;
		send_io_cnt_	= 0;
		accept_io_cnt_	= 0;

		recv_ring_buffer_.Init();
		send_ring_buffer_.Init();
	}

	Connection::~Connection()
	{
		socket_			 = INVALID_SOCKET;
		socket_listener_ = INVALID_SOCKET;
	}

	bool Connection::CreateConnection(InitConfig& config)
	{
		index_			 = config.index_;
		socket_listener_ = config.sock_listener_;
		//recv_overlappedex_.SetConnection(this);
		//send_overlappedex_.SetConnection(this);
		recv_ring_buffer_.Create(config.recv_buf_size_ * config.recv_buf_cnt_);
		send_ring_buffer_.Create(config.send_buf_size_ * config.send_buf_cnt_);

		recv_buf_size_ = config.recv_buf_size_;
		send_buf_size_ = config.send_buf_size_;

		return BindAcceptEx();
	}

	bool Connection::BindAcceptEx()
	{
		DWORD bytes;
		memset(&recv_overlappedex_.overlapped_, 0, sizeof WSAOVERLAPPED);
		recv_overlappedex_.wsabuf_.buf	= address_;
		recv_overlappedex_.wsabuf_.len	= recv_buf_size_;		
		recv_overlappedex_.msg_			= address_;
		recv_overlappedex_.iomode_		= IoMode::IO_ACCEPT;
		recv_overlappedex_.connection_	= this;
		socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP,
			nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (socket_ == INVALID_SOCKET)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | WSASocket() failed : Error[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}

		IncrementAcceptIoCount();
		bool result = AcceptEx(socket_listener_,
			socket_, 
			recv_overlappedex_.wsabuf_.buf,
			0, 
			sizeof SOCKADDR_IN + 16,
			sizeof SOCKADDR_IN + 16,
			&bytes,
			&recv_overlappedex_.overlapped_);
		if (result == false && WSAGetLastError() != WSA_IO_PENDING)
		{
			DecrementAcceptIoCount();
			logger_.Write(LogType::L_ERROR, "System | %s | AcceptEx() failed : Error[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}
		return true;
	}
	
	bool Connection::BindIocp(HANDLE iocp)
	{
		LockGurad lock(cs_);
		if (iocp == nullptr)
		{
			return false;
		}
		HANDLE result_handle = CreateIoCompletionPort((HANDLE)socket_, iocp, reinterpret_cast<ULONG_PTR>(this), 0);
		if (iocp == nullptr || iocp != result_handle)
		{
			logger_.Write(LogType::L_ERROR, "System | Socket:%d | %s | CreateIoCompletionPort() failed : Error[%d]", socket_, __FUNCTION__, WSAGetLastError());
			return false;
		}
		iocp_ = iocp;
		return true;
	}

	bool Connection::CloseConnection(bool is_forced)
	{
		LockGuard lock(cs_);
		linger li = { 0, 0 };
		if (is_forced == true)
		{
			li.l_onoff = 1;
		}
		// ��ȣ ������ �� �ϴ� ���� ����
		// IocpServer �ʿ��� Connection ���� ��ƾ ����
		//if (IocpServer() != nullptr && m_bIsConnect)
		//	IocpServer()->OnClose(this);

		shutdown(socket_, SD_BOTH);
		setsockopt(socket_, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&li), sizeof li);
		closesocket(socket_);
		socket_ = INVALID_SOCKET;

		recv_overlappedex_.still_ = 0;
		recv_overlappedex_.total_bytes_ = 0;

		send_overlappedex_.still_ = 0;
		send_overlappedex_.total_bytes_ = 0;

		Init();
		BindAcceptEx();
		return true;
	}

	char* Connection::PrepareSendPacket(int len)
	{
		if (is_connected_ == false)
		{
			return nullptr;
		}
		char* buf = send_ring_buffer_.ForwardMark(len);
		if (buf == nullptr)
		{
			// �����÷ο��ε� �Ϲ����� ��Ȳ�̶��
			// ������ ���� ����Ʈ ���̹Ƿ� ������ ���´�
			// IocpServer()->CloseConnection(this);
			CloseConnection();
			logger_.Write(LogType::L_ERROR, "System | Socket:%d | %s | Send Ring buffer ForwardMark() failed : Error[%d]", socket_, __FUNCTION__,WSAGetLastError());
			return nullptr;
		}
		memset(buf, 0, len);
		memcpy(buf, &len, PACKET_LENGTH_BYTE);
		return buf;
	}

	bool Connection::ReleaseSendPacket(OverlappedEx* send_overlappedex)
	{
		if (send_overlappedex == nullptr)
		{
			return false;
		}

		send_ring_buffer_.ReleaseBuffer(send_overlappedex_.wsabuf_.len);
		return true;
	}

	// ������ remain�� �� �����ؾ��� ���� �ִ� ����Ʈ�� �ƴ϶�
	// ������� ������ ����Ʈ ���� Ȯ���� ����. �׷� ��� still�� ����
	bool Connection::RecvPost(char* next, DWORD still)
	{
		DWORD	flag = 0;
		DWORD	recv_bytes = 0;
		assert(is_connected_ == true);
		recv_overlappedex_.iomode_ = IoMode::IO_RECV;
		recv_overlappedex_.still_ = still;

		// fuck that shit. ���� server �� ���� �ٽ� ������
		// ���� ���̾� �̰� ��ü ��ģ ������ ������
		int move = still - (recv_ring_buffer_.GetWriteMark() - next);
		recv_overlappedex_.wsabuf_.buf =
			recv_ring_buffer_.ForwardMark(move, recv_buf_size_, still);

		if (recv_overlappedex_.wsabuf_.buf == nullptr)
		{
			CloseConnection();
			logger_.Write(LogType::L_ERROR, "System | Socket:%d | %s | Recv Ring buffer ForwardMark() failed", socket_, __FUNCTION__);
			return false;
		}
		// �� �ڵ�� �� ����
		//recv_overlappedex_.msg_ = recv_overlappedex_.wsabuf_.buf - still;
		memset(&recv_overlappedex_.overlapped_, 0, sizeof WSAOVERLAPPED);
		IncrementRecvIoCount();
		auto result = WSARecv(
			socket_,
			&recv_overlappedex_.wsabuf_,
			1,
			&recv_bytes,
			&flag,
			&recv_overlappedex_.overlapped_,
			nullptr
		);

		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			DecrementRecvIoCount();
			// IocpServer()->CloseConnection(this);
			CloseConnection();
			logger_.Write(LogType::L_ERROR, "System | Socket:%d | %s | WSARecv() failed: Error[%u]", 
				socket_, __FUNCTION__, WSAGetLastError());
			return false;
		}
		return true;
	}

	bool Connection::SendPost()
	{
		DWORD bytes = 0;
		if (InterlockedCompareExchange(reinterpret_cast<long*>(&is_sent_), FALSE, TRUE))
		{
			int read_size = 0;
			char* buf = send_ring_buffer_.GetBuffer(send_buf_size_, read_size);
			if (buf == nullptr)
			{
				InterlockedExchange(reinterpret_cast<long*>(&is_sent_), TRUE);
				return false;
			}

			// �� �������� �� ������ �� ��
			send_overlappedex_.still_ = 0;
			send_overlappedex_.iomode_ = IoMode::IO_SEND;
			send_overlappedex_.total_bytes_ = read_size;
			memset(&send_overlappedex_.overlapped_, 0, sizeof WSAOVERLAPPED);
			send_overlappedex_.wsabuf_.len = read_size;
			send_overlappedex_.wsabuf_.buf = buf;
			send_overlappedex_.connection_ = this;

			IncrementSendIoCount();
			auto result = WSASend(
				socket_,
				&send_overlappedex_.wsabuf_,
				1,
				&bytes,
				0,
				&send_overlappedex_.overlapped_,
				nullptr
			);
			if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				DecrementSendIoCount();
				// IocpServer()->CloseConnection(this);
				CloseConnection();
				logger_.Write(LogType::L_ERROR, "System | Socket:%d | %s | WSASend() failed: Error[%u]",
					socket_, __FUNCTION__, WSAGetLastError());
				InterlockedExchange(reinterpret_cast<long*>(&is_sent_), FALSE);
				return false;
			}
		}
		return true;
	}

	int Connection::DecrementRecvIoCount()
	{
		LockGurad lock(cs_);
		return recv_io_cnt_ ? InterlockedDecrement(&recv_io_cnt_) : 0;
	}
	int Connection::DecrementSendIoCount()
	{
		LockGurad lock(cs_);
		return send_io_cnt_ ? InterlockedDecrement(&send_io_cnt_) : 0;
	}
	int Connection::DecrementAcceptIoCount()
	{
		LockGurad lock(cs_);
		return accept_io_cnt_ ? InterlockedDecrement(&accept_io_cnt_) : 0;
	}
}
