#include "stdafx.h"
//#include "IocpServer.h"
//#include "ILog.h"

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
		assert(bresult == true);

		bresult = BindListenSocketIocp();
		assert(bresult == true);

		bresult = CreateSessions();
		assert(bresult == true);

		bresult = CreateWorkerThread();
		assert(bresult == true);

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
		DestroySessions();

		Log->Write(LogType::L_INFO, "Server ended");
	}	

	void IocpServer::SendPacket(const int sessionIndex, const void* packet, const short packetSize)
	{
		auto session = GetSession(sessionIndex);
		if (session == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | GetSession() failure", __FUNCTION__);
			return;
		}

		char* sendBufReserved = nullptr;
		auto result = session->ReserveSendPacketBuffer(&sendBufReserved, packetSize);
		if (result == Result::RESERVED_BUFFER_NOT_CONNECTED)
		{
			Log->Write(LogType::L_ERROR, "%s | Not connected failure", __FUNCTION__);
			return;
		}
		else if (result == Result::RESERVED_BUFFER_EMPTY)
		{
			if (session->CloseCompletely())
			{
				HandleSessionCloseException(session);
			}

			Log->Write(LogType::L_ERROR, "%s | RingSendBuffer.ForwardMark() failure", __FUNCTION__);
			return;
		}
		memcpy(sendBufReserved, packet, packetSize);

		if (session->PostSend(packetSize, sendBufReserved) == false)
		{
			if (session->CloseCompletely())
			{
				HandleSessionCloseException(session);
			}
		}
	}

	Result IocpServer::CreateListenSocket()
	{
		WSADATA wsadata;
		if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		{
			Log->Write(LogType::L_ERROR, "%s | WSAStartup() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_WSASTARTUP;
		}

		ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (ListenSocket == INVALID_SOCKET)
		{
			Log->Write(LogType::L_ERROR, "%s | WSASocket() failure: error[%d]", __FUNCTION__, WSAGetLastError());
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
			Log->Write(LogType::L_ERROR, "%s | CreateIoCompletionPort() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}
		return true;
	}

	bool IocpServer::CreateSessions()
	{
		SessionConfig config;
		config.MaxRecvBufferSize = ServerInitConfig->SessionMaxRecvBufferSize;
		config.MaxSendBufferSize = ServerInitConfig->SessionMaxSendBufferSize;
		config.MaxPacketSize	 = ServerInitConfig->MaxPacketSize;

		for (int i = 0; i < ServerInitConfig->MaxSessionCount; ++i)
		{
			auto session = new Session();
			session->Init(ListenSocket, i, &config, Log);
			SessionVector.push_back(session);
		}
		return true;
	}

	void IocpServer::DestroySessions()
	{
		for (int i = 0; i < ServerInitConfig->MaxSessionCount; ++i)
		{
			delete SessionVector[i];
		}
	}

	Session* IocpServer::GetSession(const int connectionIndex)
	{
		if (connectionIndex < 0 || connectionIndex >= ServerInitConfig->MaxSessionCount)
		{
			return nullptr;
		}
		return SessionVector[connectionIndex];
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
			Session* session = nullptr;

			auto result = GetQueuedCompletionStatus(
				WorkerIocp,	&bytes,
				reinterpret_cast<PULONG_PTR>(&session),
				reinterpret_cast<OVERLAPPED**>(&overlappedEx),
				INFINITE
			);

			if (!result || (0 == bytes && IoMode::ACCEPT != overlappedEx->Mode))
			{
				HandleWorkerThreadException(session, overlappedEx);
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

	bool IocpServer::ProcessIocpMessage(OUT char& msgType, OUT int& sessionIndex, OUT char** buf, OUT short& copySize, int waitMilliseconds)
	{
		Message* msg = nullptr;
		Session* session = nullptr;
		DWORD bytes = 0;
		if (waitMilliseconds == 0)
		{
			waitMilliseconds = INFINITE;
		}

		auto result = GetQueuedCompletionStatus(
			LogicIocp, &bytes,
			reinterpret_cast<PULONG_PTR>(&session),
			reinterpret_cast<OVERLAPPED**>(&msg),
			waitMilliseconds
		);
		if (result == false)
		{
			this_thread::sleep_for(chrono::milliseconds(1));
			return false;
		}

		switch (msg->Type)
		{
		case MessageType::CONNECTION:
		{
			DoPostConnection(session, msg, msgType, sessionIndex);
			break;
		}

		case MessageType::CLOSE:
		{
			DoPostClose(session, msg, msgType, sessionIndex);
			break;
		}
		case MessageType::ONRECV:
		{
			DoPostRecvPacket(session, msg, msgType, sessionIndex, buf, copySize, bytes);
			UniqueMessagePool.get()->DeallocateMsg(msg);
			break;
		}
		}
		return true;
	}

	Result IocpServer::PostMessageToQueue(Session* session, Message* msg, const DWORD packetSize)
	{
		if (LogicIocp == INVALID_HANDLE_VALUE || msg == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | PostMessageToQueue() failure", __FUNCTION__);
			return Result::FAIL_MESSAGE_NULL;
		}

		auto result = PostQueuedCompletionStatus(
			LogicIocp, packetSize,
			reinterpret_cast<ULONG_PTR>(session),
			reinterpret_cast<OVERLAPPED*>(msg)
		);

		if (!result)
		{
			Log->Write(LogType::L_ERROR, "%s | PostQueuedCompletionStatus() failure[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_PQCS;
		}
		return Result::SUCCESS;
	}

	void IocpServer::HandleWorkerThreadException(Session* session, const OverlappedEx* overlappedEx)
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
				session->DecrementAcceptIoCount();
				break;
			}
			case IoMode::RECV:
			{
				session->DecrementRecvIoCount();
				break;
			}
			case IoMode::SEND:
			{
				session->DecrementSendIoCount();
				break;
			}
		}

		if (session->CloseCompletely())
		{
			HandleSessionCloseException(session);
		}
		return;
	}

	void IocpServer::HandleSessionCloseException(Session* session)
	{
		if (session == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | session is nullptr", __FUNCTION__);
			return;
		}

		if (PostMessageToQueue(session, session->GetCloseMsg()) != Result::SUCCESS)
		{
			session->ResetSession();
		}
	}

	void IocpServer::DoAccept(const OverlappedEx* overlappedEx)
	{
		auto session = GetSession(overlappedEx->SessionIndex);
		if (session == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | session is nullptr", __FUNCTION__);
			return;
		}
		session->DecrementAcceptIoCount();

		if (session->SetAddressInfo() == false)
		{
			Log->Write(LogType::L_ERROR, "%s | GetAcceptExSockaddrs() failure[%d]", __FUNCTION__, WSAGetLastError());
			if (session->CloseCompletely())
			{
				HandleSessionCloseException(session);
			}
			return;
		}

		if (!session->BindIocp(WorkerIocp))
		{
			if (session->CloseCompletely())
			{
				HandleSessionCloseException(session);
			}
			return;
		}
		session->SetStateConnected();

		auto result = session->PostRecv(0, 0);
		if (result != Result::SUCCESS)
		{
			Log->Write(LogType::L_ERROR, "%s | PostRecv() failure[%d]", __FUNCTION__, WSAGetLastError());
			HandleSessionCloseException(session);
			return;
		}

		if (PostMessageToQueue(session, session->GetConnectionMsg()) != Result::SUCCESS)
		{
			session->Disconnect();
			session->ResetSession();
			return;
		}
	}

	void IocpServer::DoRecv(OverlappedEx* overlappedEx, const DWORD size)
	{
		Session* session = GetSession(overlappedEx->SessionIndex);
		if (session == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | session is nullptr", __FUNCTION__);
			return;
		}

		session->DecrementRecvIoCount();
		overlappedEx->Remain += size; 

		auto remain = overlappedEx->Remain;
		auto forwardLength = overlappedEx->Remain;
		auto buf = overlappedEx->Wsabuf.buf;

		ForwardPacket(session, remain, buf);
		if (session->PostRecv(forwardLength, remain) != Result::SUCCESS)
		{
			if (session->CloseCompletely())
			{
				HandleSessionCloseException(session);
			}
		}
	}

	void IocpServer::ForwardPacket(Session* session, DWORD& remain, char* buf)
	{
		short packetSize = 0;

		while (true)
		{
			if (remain < kPacketHeaderLength)
			{
				break;
			}
			memcpy(&packetSize, buf, kPacketSizeLength);
			auto currentSize = packetSize;

			if (currentSize <= 0 || currentSize > session->GetRecvBufferSize())
			{
				Log->Write(LogType::L_ERROR, "%s | Wrong packet is received", __FUNCTION__);
				if (session->CloseCompletely())
				{
					HandleSessionCloseException(session);
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
				if (PostMessageToQueue(session, msg, packetSize) != Result::SUCCESS)
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
		Session* session = GetSession(overlappedEx->SessionIndex);
		if (session == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | session is nullptr", __FUNCTION__);
			return;
		}

		session->DecrementSendIoCount();
		overlappedEx->Remain += size;

		// 모든 메시지를 전송하지 못했을 때
		if (static_cast<DWORD>(overlappedEx->TotalBytes) > overlappedEx->Remain)
		{
			Log->Write(LogType::L_ERROR, "%s | 전송 진행 중", __FUNCTION__);
			session->IncrementSendIoCount();
			overlappedEx->Wsabuf.buf += size;
			overlappedEx->Wsabuf.len -= size;
			memset(&overlappedEx->Overlapped, 0, sizeof OVERLAPPED);

			DWORD bytes = 0;
			auto result = WSASend(
				session->GetClientSocket(),
				&overlappedEx->Wsabuf, 1,
				&bytes, 0, 
				&overlappedEx->Overlapped, nullptr
			);

			if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				session->DecrementSendIoCount();
				Log->Write(LogType::L_ERROR, "%s | WSASend() failure[%d]", __FUNCTION__, WSAGetLastError());
				if (session->CloseCompletely())
				{
					HandleSessionCloseException(session);
				}
				return;
			}
		}
		// 모든 메시지 전송
		else
		{
			session->SetSendAvaliable();
		}
	}

	void IocpServer::DoPostConnection(Session* session, const Message* msg, OUT char& msgType, OUT int& sessionIndex)
	{
		if (session->IsConnected() == false)
		{
			Log->Write(LogType::L_ERROR, "%s | Not connected", __FUNCTION__);
			return;
		}

		msgType = static_cast<char>(msg->Type);
		sessionIndex = session->GetIndex();
	}

	void IocpServer::DoPostClose(Session* session, const Message* msg, OUT char& msgType, OUT int& sessionIndex)
	{
		msgType = static_cast<char>(msg->Type);
		sessionIndex = session->GetIndex();
		session->ResetSession();
	}

	void IocpServer::DoPostRecvPacket(Session* session, const Message* msg, OUT char& msgType, OUT int& sessionIndex, OUT char** buf, OUT short& copySize, const DWORD size)
	{
		if (msg->Contents == nullptr)
		{
			Log->Write(LogType::L_ERROR, "%s | Message contents is nullptre", __FUNCTION__);
			return;
		}

		msgType = static_cast<char>(msg->Type);
		sessionIndex = session->GetIndex();
		copySize = static_cast<short>(size);
		*buf = msg->Contents;
	}
}