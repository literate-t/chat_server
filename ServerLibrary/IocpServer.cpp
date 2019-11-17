#include "stdafx.h"

namespace ServerLibrary
{
	void IocpServer::Init(ServerConfig* config, ILog* log)
	{
		ServerInitConfig = config;
		Log = log;
	}

	bool IocpServer::Start()
	{
		auto result = CreateListenSocket();
		assert(result == Result::SUCCESS);

		result = CreateIocp();
		assert(result == Result::SUCCESS);

		auto bresult = CreateMessagePool();
		assert(bresult == false);

		bresult = BindListenSocketIocp();
		assert(bresult == false);

		bresult = CreateConnections();
		assert(bresult == false);

		bresult = CreatePerformance();
		assert(bresult == false);

		bresult = CreateWorkerThread();
		assert(bresult == false);

		Log->Write(LogType::L_INFO, "Server started");
		return true;
	}

	void IocpServer::End()
	{
		if (WorkerIocp != INVALID_HANDLE_VALUE)
		{
			IsWorkerThreadRunnig = false;
			CloseHandle(WorkerIocp);

			for (int i = 0; i < WorkerThreadVector.size(); ++i)
			{
				if (WorkerThreadVector[i].get()->joinable())
				{
					WorkerThreadVector[i].get()->join();
				}
			}
		}

		if (LogicIocp != INVALID_HANDLE_VALUE)
		{
			CloseHandle(LogicIocp);
		}

		if (ListenSocket != INVALID_SOCKET)
		{
			closesocket(ListenSocket);
			ListenSocket = INVALID_SOCKET;
		}
		WSACleanup();
		DestroyConnections();

		Log->Write(LogType::L_INFO, "Server ended");
	}

	bool IocpServer::ProcessIocpMessage(OUT char& msgType, OUT int& connectionIndex, char* buf, OUT short& copySize, int waitMilliseconds)
	{
		Message* msg = nullptr;
		Connection* connection = nullptr;
		DWORD bytes = 0;
		if (waitMilliseconds == 0)
		{
			waitMilliseconds = INFINITE;
		}

		auto result = GetQueuedCompletionStatus(
			LogicIocp, &bytes,
			reinterpret_cast<PULONG_PTR>(&connection),
			reinterpret_cast<OVERLAPPED**>(&msg),
			waitMilliseconds
		);
		// assert(result == true);
		if (result == false)
		{
			Log->Write(LogType::L_ERROR, "%s | GetQueuedCompletionStatus() failure[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}

		switch (msg->Type)
		{
			case MessageType::CONNECTION:
			{
				DoPostConnection(connection, msg, msgType, connectionIndex);
				break;
			}
			// 재사용에 딜레이를 주지 않으면 재사용하기 전에 워커 스레드에서 이 세션이 호출될 수도
			case MessageType::CLOSE:
			{
				DoPostClose(connection, msg, msgType, connectionIndex);
				break;
			}
			case MessageType::ONRECV:
			{
				DoPostRecvPacket(connection, msg, msgType, connectionIndex, buf, copySize, bytes);
				UniqueMessagePool.get()->DeallocateMsg(msg);
				break;
			}
		}
		return true;
	}

	void IocpServer::SendPacket(const int connectionIndex, const void* packet, const short packetSize)
	{
		auto connection = GetConnection(connectionIndex);
		if (connection == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | GetConnection() failure", __FUNCTION__);
			return;
		}

		char* sendBufReserved = nullptr;
		auto result = connection->ReserveSendPacketBuffer(&sendBufReserved, packetSize);
		if (result == Result::RESERVED_BUFFER_NOT_CONNECTED)
		{
			Log->Write(LogType::L_ERROR, "%s | Not connected failure", __FUNCTION__);
			return;
		}
		else if (result == Result::RESERVED_BUFFER_EMPTY)
		{
			if (!connection->CloseCompletely())
			{
				HandleConnectionCloseException(connection);
			}

			Log->Write(LogType::L_ERROR, "%s | RingSendBuffer.ForwardMark() failure", __FUNCTION__);
			return;
		}

		// 이상함. 복사하고 사용을 안 하네
		memcpy(sendBufReserved, packet, packetSize);
		if (connection->PostSend(packetSize) == false)
		{
			if (!connection->CloseCompletely())
			{
				HandleConnectionCloseException(connection);
			}
		}
	}

	Result IocpServer::CreateListenSocket()
	{
		WSADATA wsadata;
		if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		{
			Log->Write(LogType::L_ERROR, "%s | WSAStartup() failure[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_WSASTARTUP;
		}

		ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (ListenSocket == INVALID_SOCKET)
		{
			Log->Write(LogType::L_ERROR, "%s | WSASocket() failure[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_CREATE_LISTENSOCKET;
		}

		SOCKADDR_IN addrIn;
		memset(&addrIn, 0, sizeof SOCKADDR_IN);
		addrIn.sin_family = AF_INET;
		addrIn.sin_port = htons(ServerInitConfig->Port);
		addrIn.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(ListenSocket, (sockaddr*)&addrIn, sizeof addrIn) == SOCKET_ERROR)
		{
			Log->Write(LogType::L_ERROR, "%s | bind() failure[%d]", __FUNCTION__, GetLastError());
			return Result::FAIL_BIND_LISTENSOCKET;
		}

		if (listen(ListenSocket, ServerInitConfig->BackLogCount) == SOCKET_ERROR)
		{
			Log->Write(LogType::L_ERROR, "%s | listen() failure[%d]", __FUNCTION__, GetLastError());
			return Result::FAIL_LISTEN_LISTENSOCKET;
		}
		return Result::SUCCESS;
	}

	Result IocpServer::CreateIocp()
	{
		WorkerIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
		if (WorkerIocp == INVALID_HANDLE_VALUE)
		{
			Log->Write(LogType::L_ERROR, "%s | CreateIoCompletionPort() failure[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_CREATE_WORKER_IOCP;
		}

		LogicIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
		if (LogicIocp == INVALID_HANDLE_VALUE)
		{
			Log->Write(LogType::L_ERROR, "%s | CreateIoCompletionPort() failure[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_CREATE_LOGIC_IOCP;
		}
		return Result::SUCCESS;
	}

	bool IocpServer::CreateMessagePool()
	{
		UniqueMessagePool = make_unique<MessagePool>(ServerInitConfig->MaxMessagePoolCount, ServerInitConfig->ExtraMessagePoolCount);
		UniqueMessagePool->SetLog(Log);
		if (!UniqueMessagePool->CheckCounts())
		{
			return false;
		}
		return true;
	}

	bool IocpServer::BindListenSocketIocp()
	{
		auto iocp = CreateIoCompletionPort(reinterpret_cast<HANDLE>(ListenSocket), WorkerIocp, 0, 0);
		if (iocp == INVALID_HANDLE_VALUE || iocp != WorkerIocp)
		{
			Log->Write(LogType::L_ERROR, "%s | CreateIoCompletionPort() failure[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}
		return true;
	}

	bool IocpServer::CreateConnections()
	{
		ConnectionConfig config;
		config.MaxRecvBufferSize = ServerInitConfig->ConnectionMaxRecvBufferSize;
		config.MaxSendBufferSize = ServerInitConfig->ConnectionMaxSendBufferSize;
		config.MaxRecvOverlappedBufferSize = ServerInitConfig->MaxRecvOverlappedBufferSize;
		config.MaxSendOverlappedBufferSize = ServerInitConfig->MaxSendOverlappedBufferSize;

		for (int i = 0; i < ServerInitConfig->MaxConnectionCount; ++i)
		{
			auto connection = new Connection();
			connection->Init(ListenSocket, i, &config, Log);
			VectorConnection.push_back(connection);
		}
		return true;
	}

	void IocpServer::DestroyConnections()
	{
		for (int i = 0; i < ServerInitConfig->MaxConnectionCount; ++i)
		{
			delete VectorConnection[i];
		}
	}

	Connection* IocpServer::GetConnection(const int connectionIndex)
	{
		if (connectionIndex < 0 || connectionIndex >= ServerInitConfig->MaxConnectionCount)
		{
			return nullptr;
		}
		return VectorConnection[connectionIndex];
	}

	bool IocpServer::CreatePerformance()
	{
		if (ServerInitConfig->PerformancePacketMillisecondsTime == -1)
		{
			return false;
		}
		UniquePerformance = make_unique<Performance>();
		UniquePerformance->SetLog(Log);
		UniquePerformance->Start(ServerInitConfig->PerformancePacketMillisecondsTime);
		return true;
	}

	bool IocpServer::CreateWorkerThread()
	{
		if (ServerInitConfig->WorkerThreadCount == -1)
		{
			return false;
		}

		for (int i = 0; i < ServerInitConfig->WorkerThreadCount; ++i)
		{
			WorkerThreadVector.push_back(make_unique<thread>([&]() {WorkerThread(); }));
		}
		return true;
	}

	void IocpServer::WorkerThread()
	{
		while (IsWorkerThreadRunnig)
		{
			DWORD bytes = 0;
			OverlappedEx* overlappedEx = nullptr;
			Connection* connection = nullptr;

			auto result = GetQueuedCompletionStatus(
				WorkerIocp,	&bytes,
				reinterpret_cast<PULONG_PTR>(&connection),
				reinterpret_cast<OVERLAPPED**>(&overlappedEx),
				INFINITE
			);

			if (!result || (0 == bytes && IoMode::ACCEPT != overlappedEx->Mode))
			{
				HandleWorkerThreadException(connection, overlappedEx);
				continue;
			}

			if (overlappedEx == nullptr)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					Log->Write(LogType::L_ERROR, "%s | GetQueuedCompletionStatus() failure[%d]", __FUNCTION__, WSAGetLastError());
					continue;
				}
				Log->Write(LogType::L_ERROR, "%s | overlappedEx is nullptr[%d]", __FUNCTION__, WSAGetLastError());
				continue;
			}

			switch (overlappedEx->Mode)
			{
				case IoMode::ACCEPT:
				{
					DoAccept(overlappedEx);
					break;
				}
				case IoMode::RECV:
				{
					DoRecv(overlappedEx, bytes);
					break;
				}
				case IoMode::SEND:
				{
					DoSend(overlappedEx, bytes);
					break;
				}
			}
		}
	}

	Result IocpServer::PostMessageToQueue(Connection* connection, Message* msg, const DWORD packetSize)
	{
		if (LogicIocp == INVALID_HANDLE_VALUE || msg == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | PostMessageToQueue() failure", __FUNCTION__);
			return Result::FAIL_MESSAGE_NULL;
		}

		auto result = PostQueuedCompletionStatus(
			LogicIocp, packetSize,
			reinterpret_cast<ULONG_PTR>(connection),
			reinterpret_cast<OVERLAPPED*>(msg)
		);

		if (!result)
		{
			Log->Write(LogType::L_ERROR, "%s | PostQueuedCompletionStatus() failure[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_PQCS;
		}
		return Result::SUCCESS;
	}

	void IocpServer::HandleWorkerThreadException(Connection* connection, const OverlappedEx* overlappedEx)
	{
		if (overlappedEx == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | overlappedEx is nullptr", __FUNCTION__);
			return;
		}

		switch (overlappedEx->Mode)
		{
			case IoMode::ACCEPT:
			{
				connection->DecrementAcceptIoCount();
				break;
			}
			case IoMode::RECV:
			{
				connection->DecrementRecvIoCount();
				break;
			}
			case IoMode::SEND:
			{
				connection->DecrementSendIoCount();
				break;
			}
		}

		if (!connection->CloseCompletely())
		{
			HandleConnectionCloseException(connection);
		}
		return;
	}

	void IocpServer::HandleConnectionCloseException(Connection* connection)
	{
		if (connection == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | connection is nullptr", __FUNCTION__);
			return;
		}

		connection->Disconnect();
		connection->SetStateDisconnected();

		if (PostMessageToQueue(connection, connection->GetCloseMsg()) != Result::SUCCESS)
		{
			connection->ResetConnection();
		}
	}

	void IocpServer::DoAccept(const OverlappedEx* overlappedEx)
	{
		auto connection = GetConnection(overlappedEx->ConnectionIndex);
		if (connection == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | connection is nullptr", __FUNCTION__);
			return;
		}
		connection->DecrementAcceptIoCount();

		if (connection->SetAddressInfo() == false)
		{
			Log->Write(LogType::L_ERROR, "%s | GetAcceptExSockaddrs() failure[%d]", __FUNCTION__, WSAGetLastError());
			if (!connection->CloseCompletely())
			{
				HandleConnectionCloseException(connection);
			}
			return;
		}

		if (!connection->BindIocp(WorkerIocp))
		{
			if (!connection->CloseCompletely())
			{
				HandleConnectionCloseException(connection);
			}
			return;
		}
		connection->SetStateConnected();

		auto result = connection->PostRecv(connection->GetRecvBufferBegin(), 0);
		if (result != Result::SUCCESS)
		{
			Log->Write(LogType::L_ERROR, "%s | PostRecv() failure[%d]", __FUNCTION__, WSAGetLastError());
			HandleConnectionCloseException(connection);
			return;
		}

		if (PostMessageToQueue(connection, connection->GetConnectionMsg()) != Result::SUCCESS)
		{
			connection->Disconnect();
			connection->ResetConnection();
			return;
		}
	}

	void IocpServer::DoRecv(OverlappedEx* overlappedEx, const DWORD size)
	{
		Connection* connection = GetConnection(overlappedEx->ConnectionIndex);
		if (connection == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | connection is nullptr", __FUNCTION__);
			return;
		}

		connection->DecrementRecvIoCount();
		overlappedEx->Wsabuf.buf = overlappedEx->SocketMsg;
		overlappedEx->Remain += size; 

		auto remain = overlappedEx->Remain; 
		auto buf = overlappedEx->Wsabuf.buf;

		ForwardPacket(connection, remain, buf);
		if (connection->PostRecv(buf, remain) != Result::SUCCESS)
		{
			if (!connection->CloseCompletely())
			{
				HandleConnectionCloseException(connection);
			}
		}
	}

	void IocpServer::ForwardPacket(Connection* connection, DWORD& remain, char* buf)
	{
		const int kPacketHeaderLength = 4;
		const int kPacketSizeLength = 2;
		const int kPacketTypeLength = 2;
		short packetSize = 0;

		while (true)
		{
			if (remain < kPacketHeaderLength)
			{
				break;
			}
			memcpy(&packetSize, buf, kPacketSizeLength);
			auto currentSize = packetSize;

			if (currentSize <= 0 || currentSize > connection->GetRecvBufferSize())
			{
				Log->Write(LogType::L_ERROR, "%s | Wrong packet is received", __FUNCTION__);
				if (!connection->CloseCompletely())
				{
					HandleConnectionCloseException(connection);
				}
				return;
			}

			if (remain >= static_cast<DWORD>(currentSize))
			{
				auto msg = UniqueMessagePool->AllocateMsg();
				if (msg == nullptr)
				{
					Log->Write(LogType::L_ERROR, "%s | Message is empty", __FUNCTION__);
					return;
				}

				msg->SetMessagae(MessageType::ONRECV, buf);
				if (PostMessageToQueue(connection, msg, packetSize) != Result::SUCCESS)
				{
					UniqueMessagePool->DeallocateMsg(msg);
					return;
				}
				remain -= currentSize;
				buf += currentSize;
			}
			else
			{
				break;
			}
		}
	}

	void IocpServer::DoSend(OverlappedEx* overlappedEx, const DWORD size)
	{
		Connection* connection = GetConnection(overlappedEx->ConnectionIndex);
		if (connection == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | connection is nullptr", __FUNCTION__);
			return;
		}

		connection->DecrementSendIoCount();
		overlappedEx->Remain += size;

		// 모든 메시지를 전송하지 못했을 때
		if (static_cast<DWORD>(overlappedEx->TotalBytes) > overlappedEx->Remain)
		{
			connection->IncrementSendIoCount();
			overlappedEx->Wsabuf.buf += size;
			overlappedEx->Wsabuf.len -= size;
			memset(&overlappedEx->Overlapped, 0, sizeof OVERLAPPED);

			DWORD bytes = 0;
			auto result = WSASend(
				connection->GetClientSocket(),
				&overlappedEx->Wsabuf, 1,
				&bytes, 0, 
				&overlappedEx->Overlapped, nullptr
			);

			if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				connection->DecrementSendIoCount();
				Log->Write(LogType::L_ERROR, "%s | WSASend() failure[%d]", __FUNCTION__, WSAGetLastError());
				if (!connection->CloseCompletely())
				{
					HandleConnectionCloseException(connection);
				}
				return;
			}
		}
		else
		{
			// ??
			connection->ReleaseSendBuffer(overlappedEx->TotalBytes);
			connection->SetSendAvaliable();
			if (connection->PostSend(0) == false)
			{
				if (!connection->CloseCompletely())
				{
					HandleConnectionCloseException(connection);
				}
			}
		}
	}

	void IocpServer::DoPostConnection(Connection* connection, const Message* msg, OUT char& msgType, OUT int& connectionIndex)
	{
		if (connection->IsConnected() == false)
		{
			Log->Write(LogType::L_ERROR, "%s | Not connected", __FUNCTION__);
			return;
		}

		msgType = static_cast<char>(msg->Type);
		connectionIndex = connection->GetIndex();
		Log->Write(LogType::L_ERROR, "%s | Connection index:%d", __FUNCTION__, connectionIndex);
	}

	void IocpServer::DoPostClose(Connection* connection, const Message* msg, OUT char& msgType, OUT int& connectionIndex)
	{
		if (connection->IsConnected() == false)
		{
			Log->Write(LogType::L_ERROR, "%s | Not connected", __FUNCTION__);
			return;
		}

		msgType = static_cast<char>(msg->Type);
		connectionIndex = connection->GetIndex();
		auto result = connection->ResetConnection();
		if (result == Result::SUCCESS)
		{
			Log->Write(LogType::L_ERROR, "%s | Disconnection index:%d", __FUNCTION__, connectionIndex);
			return;
		}
		else
		{
			Log->Write(LogType::L_ERROR, "%s | ResetConnection() failure", __FUNCTION__);
			return;
		}
	}

	void IocpServer::DoPostRecvPacket(Connection* connection, const Message* msg, OUT char& msgType, OUT int& connectionIndex, char* buf, OUT short& copySize, const DWORD size)
	{
		if (msg->Contents == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | Message contents is nullptre", __FUNCTION__);
			return;
		}

		msgType = static_cast<char>(msg->Type);
		connectionIndex = connection->GetIndex();
		memcpy(buf, msg->Contents, size);
		copySize = static_cast<short>(size);
		connection->ReleaseRecvBuffer(size); // ?
		UniquePerformance.get()->IncrementPacketProcessCount();
	}
}