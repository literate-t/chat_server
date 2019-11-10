#pragma once

namespace library
{
	class IocpServer : public Lock
	{
	public:
		IocpServer();
		~IocpServer();

		bool Init();

		void Run();
		void WorkerThread();
		void ProcessThread();
		bool CloseConnection(Connection* conn);
		bool ProcessPacket(Connection* conn, char current, DWORD current_size);

		virtual bool	StartServer(InitConfig& config);
		virtual bool	ShutServer();
		SOCKET			GetListenSocket() { return socket_listener_; }
		unsigned short	GetServerPort() { return port_; }
		char*			GetServerIp() { return ip_; }
		HANDLE			GetWorkerIocp() { return worker_iocp_; }
		void DoAccept(OverlappedEx* overlappedex);
		void DoRecv(OverlappedEx* overlappedex, DWORD io_size);
		void DoSend(OverlappedEx* overlappedex, DWORD io_size);
		PacketProcess* GetPacketProcess(IoMode iomode, WPARAM wparam, LPARAM lparam);
		void ClearProcessPacket(PacketProcess* packet_process);

		/////////순수 가상 함수////////////
		// 클라이언트가 접속될 때
		virtual bool OnAccept(Connection* conn) = 0;
		// 순서성 있는 패킷을 처리할 때
		virtual bool OnRecv(Connection* conn, DWORD size, char* msg) = 0;
		// 순서성 없는 패킷을 처리할 때
		virtual bool OnRecvImmediately(Connection* conn, DWORD size, char msg) = 0;
		// 클라이언트 접속 종료
		virtual void OnClose(Connection* conn) = 0;
		// 서버에서 ProcessThread가 아닌 다른 스레드에서 받은
		// 메시지가 순서대로 처리해야 할 때
		virtual bool OnSystemMsg(Connection* conn, DWORD msg_type, LPARAM lparam) = 0;

		static IocpServer* GetIocpServer() { return IocpServer::iocp_server_;}
		static IocpServer* iocp_server_;

	private:
		bool CreateProcessThreads();
		bool CreateWorkerThreads();
		void SetProperThreadCount();
		bool CreateIocp();
		bool CreateListenSock();

		void HandleWorkerThreadException(Connection* connection, const OverlappedEx* overlappedex);

		IocpServer(const IocpServer&);
		IocpServer& operator=(const IocpServer&);

	private:
		SOCKET	socket_listener_;
		HANDLE	worker_iocp_;
		HANDLE	process_iocp_;
		
		vector<unique_ptr<thread>>	worker_thread_;
		vector<unique_ptr<thread>>  process_thread_;

		unsigned short	port_;
		char	ip_[MAX_IP_LENGTH] = { 0 };

		DWORD	time_tick_;
		DWORD	worker_thread_count_;
		DWORD	process_thread_count_;

		bool	worker_thread_flag_;
		bool	process_thread_flag_;

		PacketProcess* process_packet_;
		DWORD process_packet_cnt_;

		Logger logger_;
	};

	IocpServer* IocpServer::iocp_server_ = nullptr; 
}