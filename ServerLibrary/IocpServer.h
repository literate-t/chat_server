#pragma once
#include "stdafx.h"
//#include "Definitions.h"
//#include "Connection.h"
//#include "MessagePool.h"
//#include "Performance.h"

namespace ServerLibrary
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
		bool ProcessIocpMessage(OUT char& msgType, OUT int& sessionIndex, OUT char** buf, OUT short& copySize, int waitMilliseconds);
		void SendPacket(const int sessionIndex, const void* packet, const short packetSize);

		int GetMaxPacketSize()		{ return ServerInitConfig->MaxPacketSize; }
		int GetMaxSessionCount() { return ServerInitConfig->MaxSessionCount; }

	private:
		Result CreateListenSocket();
		Result CreateIocp();
		bool CreateMessagePool();
		bool BindListenSocketIocp();
		bool CreateSessions();
		void DestroySessions();
		Session* GetSession(const int sessionIndex);
		bool CreateWorkerThread();
		void WorkerThread();
		Result PostMessageToQueue(Session* session, Message* msg, const DWORD packetSize = 0);
		void HandleWorkerThreadException(Session* session, const OverlappedEx* overlappedEx);
		void HandleSessionCloseException(Session* session);

		void DoAccept(const OverlappedEx* overlappedEx);
		void DoRecv(OverlappedEx* overlappedEx, const DWORD size);
		void DoSend(OverlappedEx* overlappedEx, const DWORD size);
		void ForwardPacket(Session* session, DWORD& remain, char* buffer);

		void DoPostConnection(Session* session, const Message* msg, OUT char& msgType, OUT int& sessionIndex);
		void DoPostClose(Session* session, const Message* msg, OUT char& msgType, OUT int& sessionIndex);
		void DoPostRecvPacket(Session* session, const Message* msg, OUT char& msgType, OUT int& sessionIndex, OUT char** buf, OUT short& copySize, const DWORD ioSize);

	private:
		ServerConfig* ServerInitConfig = nullptr;
		ILog* Log = nullptr;

		SOCKET ListenSocket = INVALID_SOCKET;

		vector<Session*> SessionVector;
		HANDLE WorkerIocp = INVALID_HANDLE_VALUE;
		HANDLE LogicIocp = INVALID_HANDLE_VALUE;

		bool IsWorkerThreadRunnig = true;
		vector<unique_ptr<thread>> WorkerThreadVector;
		unique_ptr<MessagePool>	   UniqueMessagePool;
	};
}