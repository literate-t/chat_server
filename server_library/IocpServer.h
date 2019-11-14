#pragma once

namespace library
{
	class IocpServer// : public Lock
	{
	public:
		IocpServer() = default;
		~IocpServer() {}

	public:
		bool Start(Config& config);
		void End();
		bool ProcessIocpMessage(OUT short& ioType, OUT int& connectionIndex);
		void SendPacket(const int connectionIndex, const void* packet, const short packetSize);

		int GetMaxPacketSize()		{ return InitConfig.MaxPacketSize; }
		int GetMaxConnectinoCount() { return InitConfig.MaxConnectionCount; }

	private:
		bool CreateListenSocket();
		bool CreateIocp();
		bool CreateMessagePool();
		bool BindListenSocketIocp();
		bool CreateConnections();
		bool DestroyConnections();
		Connection* GetConnection(const int connectionIndex);
		bool CreatePerformance();
		bool CreateWorkerThread();
		bool WorkerThread();
		bool PostMessageToQueue(Connection* connection, Message* msg, const DWORD packetSize = 0);
		void HandleWorkerThreadException(Connection* connection, const OverlappedEx* overlappedEx);
		void HandleCloseConnectionException(Connection* connection, const OverlappedEx* overlappedEx);
		void DoAccept(const OverlappedEx* overlappedEx);
		void DoRecv(OverlappedEx* overlappedEx, const DWORD size);
		void DoSend(OverlappedEx* overlappedEx, const DWORD size);
		void ForwardPacket(Connection* connection, DWORD& remain, char* buffer);

		void DoPostConnection(Connection* connection, const Message* msg, OUT char& ioMode, OUT int connectionIndex);
		void DoPostClose(Connection* connection, const Message* msg, OUT char& ioMode, OUT int& connectionIndex);
		void DoPostRecvPacket(Connection* connection, const Message* msg, OUT char& ioMode, OUT int& connectionIndex, char* buf, OUT short& copySize, const DWORD ioSize);

	private:
		Config InitConfig;
		SOCKET ListenSocket = INVALID_SOCKET;

		vector<Connection*> VectorConnection;
		HANDLE WorkerIocp = nullptr;
		HANDLE LogicIocp = nullptr;

		bool IsWorkerThreadRunnig = true;
		vector<unique_ptr<thread>> VectorWorkerThread;

		unique_ptr<MessagePool> UniqueMessagePool;
		unique_ptr<Performance> UniquePerformance;
	};
}