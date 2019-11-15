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
		bool ProcessIocpMessage(OUT char& msgType, OUT int& connectionIndex, char* buf, OUT short& copySize, int waitMilliseconds);
		void SendPacket(const int connectionIndex, const void* packet, const short packetSize);

		int GetMaxPacketSize()		{ return InitConfig.MaxPacketSize; }
		int GetMaxConnectinoCount() { return InitConfig.MaxConnectionCount; }

	private:
		Result CreateListenSocket();
		Result CreateIocp();
		bool CreateMessagePool();
		bool BindListenSocketIocp();
		bool CreateConnections();
		void DestroyConnections();
		Connection* GetConnection(const int connectionIndex);
		bool CreatePerformance();
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
		Config InitConfig;
		SOCKET ListenSocket = INVALID_SOCKET;

		vector<Connection*> VectorConnection;
		HANDLE WorkerIocp = INVALID_HANDLE_VALUE;
		HANDLE LogicIocp = INVALID_HANDLE_VALUE;

		bool IsWorkerThreadRunnig = true;
		vector<unique_ptr<thread>> VectorWorkerThread;

		unique_ptr<MessagePool> UniqueMessagePool;
		unique_ptr<Performance> UniquePerformance;

		Logger Log;
	};
}