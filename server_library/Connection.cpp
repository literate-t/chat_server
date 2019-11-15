#include "stdafx.h"

namespace library
{
	void Connection::Init(const SOCKET listenSocket, const int index, const ConnectionConfig& config)
	{
		Init();

		ListenSocket = listenSocket;
		Index = index;
		RecvBufSize = config.MaxRecvBufferSize;
		SendBufSize = config.MaxSendBufferSize;

		RecvOverlappedEx = new OverlappedEx(Index);
		SendOverlappedEx = new OverlappedEx(Index);
		RingRecvBuffer.Create(RecvBufSize);
		RingSendBuffer.Create(SendBufSize);

		ConnectionMsg.Type = MessageType::CONNECTION;
		ConnectionMsg.Contents = nullptr;
		CloseMsg.Type = MessageType::CLOSE;
		CloseMsg.Contents = nullptr;

		AcceptExSocket();
	}

	void Connection::Init()
	{
		memset(Ip, 0, kMaxIpLength);

		RingRecvBuffer.Init();
		RingSendBuffer.Init();

		Connected = FALSE;
		Closed = FALSE;
		Sendable = TRUE;

		AcceptIoCount = 0;
		RecvIoCount = 0;
		SendIoCount = 0;
	}

	Result Connection::AcceptExSocket()
	{
		memset(&RecvOverlappedEx->Overlapped, 0, sizeof OVERLAPPED);

		RecvOverlappedEx->Wsabuf.buf = AddrBuf;
		RecvOverlappedEx->SocketMsg = AddrBuf;
		RecvOverlappedEx->Wsabuf.len = RecvBufSize;
		RecvOverlappedEx->Mode = IoMode::ACCEPT;
		RecvOverlappedEx->ConnectionIndex = Index;

		ClientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (ClientSocket == INVALID_SOCKET)
		{
			Log.Write(LogType::L_ERROR, "%s | WSASocket() failure:error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_WSASOCKET;
		}
		IncrementAcceptIoCount();
		DWORD bytes = 0;
		auto result = AcceptEx(
			ListenSocket,
			ClientSocket,
			RecvOverlappedEx->Wsabuf.buf,
			0,
			sizeof SOCKADDR_IN + 16,
			sizeof SOCKADDR_IN + 16,
			&bytes,
			reinterpret_cast<OVERLAPPED*>(&RecvOverlappedEx)
		);

		if (!result && WSAGetLastError() != WSA_IO_PENDING)
		{
			DecrementAcceptIoCount();
			Log.Write(LogType::L_ERROR, "%s | AcceptEx() failure:error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_ACCEPTEX;
		}
		return Result::SUCCESS;
	}

	bool Connection::CloseCompletely()
	{
		// 소켓만 종료한 채로 전부 처리될 때까지 대기?
		if (Connected && (AcceptIoCount != 0 || RecvIoCount != 0 || SendIoCount != 0))
		{
			Disconnect();
			return true;
		}

		// 한 버만 접속 종료 처리 하기 위함
		if (InterlockedCompareExchange(reinterpret_cast<long*>(&Closed), TRUE, FALSE) == static_cast<long>(FALSE))
		{
			return false;
		}

		return true;
	}

	void Connection::Disconnect(bool forced)
	{
		SetStateDisconnected();
		LockGuard lock(Cs);
		if (ClientSocket != INVALID_SOCKET)
		{
			if (forced == true)
			{
				LINGER linger = { 0 };
				linger.l_onoff = 1;
				setsockopt(ClientSocket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&linger), sizeof LINGER);
			}
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
		}
	}

	Result Connection::ResetConnection()
	{
		LockGuard lock(Cs);

		RecvOverlappedEx->Remain = 0;
		RecvOverlappedEx->TotalBytes = 0;
		SendOverlappedEx->Remain = 0;
		SendOverlappedEx->TotalBytes = 0;
		Init();
		return AcceptExSocket();
	}

	bool Connection::BindIocp(const HANDLE WorkerIocp)
	{
		LockGuard lock(Cs);
		auto iocp = CreateIoCompletionPort(
			reinterpret_cast<HANDLE>(ClientSocket),
			WorkerIocp,
			reinterpret_cast<ULONG_PTR>(this),
			0
		);
		if (iocp == INVALID_HANDLE_VALUE || iocp != WorkerIocp)
		{
			Log.Write(LogType::L_ERROR, "%s | CreateIoCompletionPort() failure:error[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}
		return true;
	}

	Result Connection::PostRecv(const char* nextBuf, const DWORD remain)
	{
		assert(Connected == TRUE && RecvOverlappedEx != nullptr);

		RecvOverlappedEx->Mode = IoMode::RECV;
		RecvOverlappedEx->Remain = remain; // 20

		// 난 진짜 씨발 링 버퍼 사용하는 부분을 의도를 모르겠다 씨발 새끼들아				
		auto move = static_cast<int>(remain - (RingRecvBuffer.GetWriteMark() - nextBuf));
		RecvOverlappedEx->Wsabuf.len = RecvBufSize;
		RecvOverlappedEx->Wsabuf.buf = RingRecvBuffer.ForwardMark(move, RecvBufSize, remain);
		assert(RecvOverlappedEx->Wsabuf.buf != nullptr);

		RecvOverlappedEx->SocketMsg = RecvOverlappedEx->Wsabuf.buf;// -remain; // 얘는 왜 또 빼냐
		memset(&RecvOverlappedEx->Overlapped, 0, sizeof WSAOVERLAPPED);
		IncrementRecvIoCount();

		DWORD flag = 0;
		DWORD recvBytes = 0;
		auto result = WSARecv(
			ClientSocket,
			&RecvOverlappedEx->Wsabuf,
			1,
			&recvBytes,
			&flag,
			&RecvOverlappedEx->Overlapped,
			nullptr
		);

		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			DecrementRecvIoCount();
			Log.Write(LogType::L_ERROR, "%s | WSARecv() failure:error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::POSTRECV_NULL_SOCKET_ERROR;
		}
		return Result::SUCCESS;
	}

	bool Connection::PostSend(const int sendSize)
	{
		// 남은 패킷이 있는가
		//if (sendSize > 0)
		//{
		//	  RingSendBuffer.SetUsedBufferSize(sendSize);
		//}

		if (InterlockedCompareExchange(reinterpret_cast<long*>(&Sendable), FALSE, TRUE))
		{
			auto size = 0;
			// 분명 문제 생긴다\
			첫 사용이라면 size가 0이 될 테니까
			auto buf = RingSendBuffer.GetBuffer(SendBufSize, size); 
			if (buf == nullptr)
			{
				InterlockedExchange(reinterpret_cast<long*>(&Sendable), TRUE);
				Log.Write(LogType::L_ERROR, "%s | RingSendBuffer.GetBuffer() failure", __FUNCTION__);
				return false;
			}

			memset(&SendOverlappedEx->Overlapped, 0, sizeof WSAOVERLAPPED);

			SendOverlappedEx->Wsabuf.buf = buf;
			SendOverlappedEx->Wsabuf.len = size;
			SendOverlappedEx->ConnectionIndex = Index;

			SendOverlappedEx->Remain = 0;
			SendOverlappedEx->TotalBytes = size;
			SendOverlappedEx->Mode = IoMode::SEND;

			IncrementSendIoCount();

			DWORD sendBytes = 0;
			auto result = WSASend(
				ClientSocket,
				&SendOverlappedEx->Wsabuf,
				1,
				&sendBytes,
				0,
				&SendOverlappedEx->Overlapped,
				nullptr
			);

			if (result = SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				DecrementSendIoCount();
				Log.Write(LogType::L_ERROR, "%s | WSASend() failure[%d]", __FUNCTION__, WSAGetLastError());
				return false;
			}
		}
		return true;
	}

	Result Connection::ReserveSendPacketBuffer(OUT char** buf, const int size)
	{
		//assert(Connected);
		if (!Connected)
		{
			*buf = nullptr;			
			return Result::RESERVED_BUFFER_NOT_CONNECTED;
		}

		*buf = RingSendBuffer.ForwardMark(size);
		if (*buf == nullptr)
		{			
			return Result::RESERVED_BUFFER_EMPTY;
		}
		return Result::SUCCESS;
	}

	bool Connection::SetAddressInfo()
	{
		SOCKADDR* localAddr		= nullptr;
		SOCKADDR* remoteAddr	= nullptr;
		int localAddrLen = 0;
		int remoteAddrLen = 0;

		GetAcceptExSockaddrs(
			AddrBuf, 0,
			sizeof SOCKADDR_IN + 16,
			sizeof SOCKADDR_IN + 16,
			&localAddr, &localAddrLen,
			&remoteAddr, &remoteAddrLen
		);

		if (remoteAddrLen != 0)
		{
			SOCKADDR_IN* remoteAddrIn = reinterpret_cast<SOCKADDR_IN*>(remoteAddr);
			if (remoteAddrIn != nullptr)
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
