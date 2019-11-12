#include "stdafx.h"

namespace library
{
	IocpServer::IocpServer()
	{
		port_ = -1;

		time_tick_ = 0;
		worker_thread_count_ = 0;
		process_thread_count_ = 0;

		worker_thread_flag_ = false;
		process_thread_flag_ = false;

		//process_packet_ = nullptr;
		//process_packet_cnt_ = 0;
	}

	IocpServer::~IocpServer()
	{
		//delete[] process_packet_;
		WSACleanup();
	}

	bool IocpServer::StartServer(InitConfig& config)
	{
		port_ = config.server_port_;
		worker_thread_count_ = config.worker_thread_cnt_;
		process_thread_count_ = config.process_thread_cnt_;

		if (Init() == false)
		{
			return false;
		}
		if (!CreateIocp())
		{
			return false;
		}
		if (!CreateWorkerThreads())
		{
			return false;
		}
		if (!CreateProcessThreads())
		{
			return false;
		}
		if (!CreateListenSock())
		{
			return false;
		}
		config.sock_listener_ = socket_listener_;
		//if (process_packet_)
		//{
		//	delete[] process_packet_;
		//}
		CreateProcessPacketPool(config.process_packet_cnt_);
		return true;
	}

	void IocpServer::CreateProcessPacketPool(int count)
	{
		for (int i = 0; i < count; ++i)
		{
			PacketProcess packet;
			packet.index_ = i;
			process_packet_pool_.push_back(packet);
			process_packet_index_pool_.push(i);
		}
	}

	int IocpServer::AllocateProcessPacketIndex()
	{
		if (process_packet_index_pool_.empty())
		{
			return -1;
		}
		int index = process_packet_index_pool_.front();
		process_packet_index_pool_.pop();
		return index;
	}

	bool IocpServer::ShutServer()
	{
		logger_.Write(LogType::L_INFO, "System | %s | 서버 종료 시작", __FUNCTION__);
		if (worker_iocp_ != nullptr)
		{
			worker_thread_flag_ = false;
			//for (DWORD i = 0; i < worker_thread_count_; ++i)
			//{
			//	// worker thread에 종료 메시지 보내기
			//	PostQueuedCompletionStatus(worker_iocp_, 0, 0, nullptr);
			//}
			CloseHandle(worker_iocp_);
			for (size_t i = 0; i < worker_thread_.size(); ++i)
			{
				if (worker_thread_[i].get()->joinable())
				{
					worker_thread_[i].get()->join();
				}
			}
			worker_iocp_ = nullptr;
		}
		if (process_iocp_ != nullptr)
		{
			process_thread_flag_ = false;
			//for (DWORD i = 0; i < process_thread_count_; ++i)
			//{
			//	PostQueuedCompletionStatus(process_iocp_, 0, 0, nullptr);
			//}
			CloseHandle(process_iocp_);
			for (size_t i = 0; i < process_thread_.size(); ++i)
			{
				if (process_thread_[i].get()->joinable())
				{
					process_thread_[i].get()->join();
				}
			}
			process_iocp_ = nullptr;
		}
	}

	bool IocpServer::Init()
	{
		socket_listener_ = INVALID_SOCKET;
		worker_iocp_ = nullptr;
		process_iocp_ = nullptr;

		WSADATA wsadata;
		if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | WSAStartup() error[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}
		return true;
	}

	void IocpServer::SetProperThreadCount()
	{
		SYSTEM_INFO sys;
		DWORD count;
		GetSystemInfo(&sys);
		count = sys.dwNumberOfProcessors;
		worker_thread_count_ = count * 2 + 1;
	}

	bool IocpServer::CreateListenSock()
	{
		SOCKADDR_IN address;
		socket_listener_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (socket_listener_ == INVALID_SOCKET)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | WSASocket() error[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}

		address.sin_family = AF_INET;
		address.sin_port = htons(port_);
		address.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(socket_listener_, (sockaddr*)&address, sizeof address) == SOCKET_ERROR)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | bind() error[%d]", __FUNCTION__, GetLastError());
			return false;
		}
		if (listen(socket_listener_, 100) == SOCKET_ERROR)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | listen() error[%d]", __FUNCTION__, GetLastError());
			return false;
		}

		auto handle = CreateIoCompletionPort((HANDLE)socket_listener_, worker_iocp_, 0, 0);
		if (handle == nullptr || handle != worker_iocp_)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | CreateIoCompletionPort() error[%d]", __FUNCTION__, GetLastError());
			return false;
		}
		return true;
	}

	bool IocpServer::CreateProcessThreads()
	{
		if (process_thread_count_ == 0)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | process_thread_count_ == 0", __FUNCTION__);
			return false;
		}
		for (int i = 0; i < process_thread_count_; ++i)
		{
			process_thread_.push_back(make_unique<thread>([&] {ProcessThread(); }));
		}
		return true;
	}

	bool IocpServer::CreateWorkerThreads()
	{
		if (worker_thread_count_ == 0)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | worker_thread_count_ == 0", __FUNCTION__);
			return false;
		}
		for (int i = 0; i < worker_thread_count_; ++i)
		{
			worker_thread_.push_back(make_unique<thread>([&] {WorkerThread(); }));
		}
		return true;
	}

	bool IocpServer::CreateIocp()
	{
		worker_iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, worker_thread_count_);
		if (worker_iocp_ == INVALID_HANDLE_VALUE)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | Create WorerIo CompletionPort() error[%d]", __FUNCTION__, GetLastError());
			return false;
		}
		process_iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, process_thread_count_);
		if (process_iocp_ == INVALID_HANDLE_VALUE)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | Create ProcessIo CompletionPort() error[%d]", __FUNCTION__, GetLastError());
			return false;
		}
		return true;
	}

	void IocpServer::WorkerThread()
	{
		bool			result = false;
		Connection*		connection	= nullptr;
		OverlappedEx*	overlapped_ex = nullptr;
		DWORD			bytes = 0;

		while (worker_thread_flag_)
		{
			result = GetQueuedCompletionStatus(
				worker_iocp_, &bytes,
				reinterpret_cast<PULONG_PTR>(&connection),
				reinterpret_cast<LPOVERLAPPED*>(&overlapped_ex), INFINITE);

			if (overlapped_ex == nullptr)
			{
				logger_.Write(LogType::L_ERROR, "System | %s | GetQueuedCompletionStatus() error[%d]", __FUNCTION__, WSAGetLastError());
				continue;
			}

			if (result == false || bytes == 0 && overlapped_ex->iomode_ != IoMode::ACCEPT)
			{
				HandleWorkerThreadException(connection, overlapped_ex);
				continue;
			}
			switch (overlapped_ex->iomode_)
			{
				case IoMode::ACCEPT:
				{
					DoAccept(overlapped_ex);
					break;
				}
				case IoMode::RECV:
				{
					DoRecv(overlapped_ex, bytes);
					break;
				}
				case IoMode::SEND:
				{
					DoSend(overlapped_ex, bytes);
					break;
				}
			}
		}
	}

	void IocpServer::HandleWorkerThreadException(Connection* connection, const OverlappedEx* overlappedex)
	{
		switch (overlappedex->iomode_)
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

		CloseConnection(connection);
	}

	bool IocpServer::CloseConnection(Connection* connection)	
	{
		if (connection == nullptr)
		{
			return false;
		}

		if (connection->GetAceeptIoCount() != 0 ||
			connection->GetRecvIoCount() != 0 ||
			connection->GetSendIoCount() != 0)
		{
			shutdown(connection->GetSocket(), SD_BOTH);
			closesocket(connection->GetSocket());
			return true;
		}

		if (InterlockedCompareExchange(reinterpret_cast<long*>(&connection->is_closed_), TRUE, FALSE) == FALSE)
		{
			auto process_packet = GetProcessPacket(IoMode::CLOSE, 0, 0);
			if (process_packet == nullptr)
			{
				return false;
			}

			if (false == PostQueuedCompletionStatus(process_iocp_, 0, reinterpret_cast<ULONG_PTR>(connection), reinterpret_cast<OVERLAPPED*>(process_packet)))
			{
				ClearProcessPacket(process_packet->index_);
				logger_.Write(LogType::L_ERROR, "System | Socket:%64u | %s | PostQueuedCompletionStatus() error[%d]", connection->GetSocket(),__FUNCTION__, WSAGetLastError());
				connection->CloseConnection(true);
			}
		}
		return true;
	}

	void IocpServer::ClearProcessPacket(int index)
	{
		process_packet_pool_[index].Init();
		ReleaseProcessPacketIndex(index);
	}

	void IocpServer::ReleaseProcessPacketIndex(int index)
	{
		process_packet_index_pool_.push(index);
	}

	PacketProcess* IocpServer::GetProcessPacket(IoMode iomode, WPARAM wparam, LPARAM lparam)
	{
		auto index = AllocateProcessPacketIndex();
		if (index == -1)
		{
			return nullptr;
		}
		auto process_packet = &process_packet_pool_[index];
		process_packet->iomode_ = iomode;
		process_packet->lparam_ = lparam;
		process_packet->wparam_ = wparam;
		return process_packet;
	}

	void IocpServer::DoAccept(OverlappedEx* overlappedex)
	{
		Connection* connection = reinterpret_cast<Connection*>(overlappedex->connection_);
		if (connection == nullptr)
		{
			return;
		}
		connection->DecrementAcceptIoCount();

		if (connection->SetAddressInfo() == false)
		{
			logger_.Write(LogType::L_ERROR, "System | %s | SetAddressInfo() error[%d]", __FUNCTION__);
			CloseConnection(connection);
			return;
		}

		if (connection->BindIocp(worker_iocp_) == false)
		{
			CloseConnection(connection);
			return;
		}
		connection->SetStateConnection();

		if (false == connection->RecvPost(connection->recv_ring_buffer_.GetBegin(), 0))
		{
			CloseConnection(connection);
			return;
		}
		OnAccept(connection);
	}

	void IocpServer::DoRecv(OverlappedEx* overlappedex, DWORD io_size)
	{
		auto connection = reinterpret_cast<Connection*>(overlappedex->connection_);
		if (connection == nullptr)
		{
			return;
		}
		connection->DecrementRecvIoCount();
	}
}