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
		bool ProcessIocpMessage(OUT char& msgType, OUT int& connectionIndex, char* buf, OUT short& copySize, int waitMilliseconds);
		void SendPacket(const int connectionIndex, const void* packet, const short packetSize);

		int GetMaxPacketSize()		{ return ServerInitConfig->MaxPacketSize; }
		int GetMaxConnectinoCount() { return ServerInitConfig->MaxConnectionCount; }

	private:
		Result CreateListenSocket();
		Result CreateIocp();
		bool CreateMessagePool();
		bool BindListenSocketIocp();
		bool CreateConnections();
		void DestroyConnections();
		Connection* GetConnection(const int connectionIndex);
		bool CreateWorkerThread();
		void WorkerThread();
		Result PostMessageToQueue(Connection* connection, Message* msg, const DWORD packetSize = 0);
		void HandleWorkerThreadException(Connection* connection, const OverlappedEx* overlappedEx);
		void HandleConnectionCloseException(Connection* connection);

		void DoAccept(const OverlappedEx* overlappedEx);
		void DoRecv(OverlappedEx* overlappedEx, const DWORD size);
		void DoSend(OverlappedEx* overlappedEx, const DWORD size);
		void ForwardPacket(Connection* connection, DWORD& remain, char* buffer);

		void DoPostConnection(Connection* connection, const Message* msg, OUT char& msgType, OUT int& connectionIndex);
		void DoPostClose(Connection* connection, const Message* msg, OUT char& msgType, OUT int& connectionIndex);
		void DoPostRecvPacket(Connection* connection, const Message* msg, OUT char& msgType, OUT int& connectionIndex, char* buf, OUT short& copySize, const DWORD ioSize);		

	private:
		ServerConfig* ServerInitConfig = nullptr;
		ILog* Log = nullptr;

		SOCKET ListenSocket = INVALID_SOCKET;

		vector<Connection*> VectorConnection;
		HANDLE WorkerIocp = INVALID_HANDLE_VALUE;
		HANDLE LogicIocp = INVALID_HANDLE_VALUE;

		bool IsWorkerThreadRunnig = true;
		vector<unique_ptr<thread>> WorkerThreadVector;

		unique_ptr<MessagePool> UniqueMessagePool;
	};
}