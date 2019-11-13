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

		BindAcceptExSocket();
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

	bool Connection::BindAcceptExSocket()
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
			return false;
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
			return false;
		}
		return true;
	}

	bool Connection::CloseCompletely()
	{
		// ���ϸ� ������ ä�� ���� ó���� ������ ���?
		if (Connected && (AcceptIoCount != 0 || RecvIoCount != 0 || SendIoCount != 0))
		{
			Disconnect();
			return false;
		}

		// �� ���� ���� ���� ó�� �ϱ� ����
		if (InterlockedCompareExchange(reinterpret_cast<long*>(&Closed), TRUE, FALSE) == static_cast<long>(FALSE))
		{
			return true;
		}

		return false;
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

	bool Connection::ResetConnection()
	{
		LockGuard lock(Cs);

		RecvOverlappedEx->Remain = 0;
		RecvOverlappedEx->TotalBytes = 0;
		SendOverlappedEx->Remain = 0;
		SendOverlappedEx->TotalBytes = 0;
		Init();
		return BindAcceptExSocket();
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

	bool Connection::PostRecv(const char* nextBuf, const DWORD remain)
	{
		assert(Connected == TRUE && RecvOverlappedEx != nullptr);

		RecvOverlappedEx->Mode = IoMode::RECV;
		RecvOverlappedEx->Remain = remain;

		// �� ��¥ ���� �� ���� ����ϴ� �κ��� �ǵ��� �𸣰ڴ� ���� �������
		auto move = static_cast<int>(remain - (RingRecvBuffer.GetWriteMark() - nextBuf));
		RecvOverlappedEx->Wsabuf.len = RecvBufSize;
		RecvOverlappedEx->Wsabuf.buf = RingRecvBuffer.ForwardMark(move, RecvBufSize, remain);
		assert(RecvOverlappedEx->Wsabuf.buf != nullptr);

		RecvOverlappedEx->SocketMsg = RecvOverlappedEx->Wsabuf.buf - remain; // ��� �� �� ����
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
			return false;
		}
		return true;
	}

	bool Connection::PostSend(const int size)
	{
		// ���� ��Ŷ�� �ִ°�
		//if (size > 0)
		//{
		//	  RingSendBuffer.SetUsedBufferSize(sendSize);
		//}

		if (InterlockedCompareExchange(reinterpret_cast<long*>(&Sendable), FALSE, TRUE))
		{
			auto size = 0;
			// �и� ���� �����\
			ù ����̶�� size�� 0�� �� �״ϱ�
			auto buf = RingSendBuffer.GetBuffer(SendBufSize, size); 
			if (buf == nullptr)
			{
				InterlockedExchange(reinterpret_cast<long*>(&Sendable), TRUE);
				Log.Write(LogType::L_ERROR, "%s | RingSendBuffer.GetBuffer() failure", __FUNCTION__);
				return false;
			}

			memset(&SendOverlappedEx->Overlapped, 0, sizeof WSAOVERLAPPED);
		}
	}
}