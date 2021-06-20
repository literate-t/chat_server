#include "stdafx.h"
//#include "IocpServer.h"
//#include "ILog.h"

namespace server_library
{
	void IocpServer::Init(ServerConfig* config, ILog* log)
	{
		server_config_ = config;
		log_ = log;
	}

	bool IocpServer::Start()
	{
		auto result = CreateListenSocket();
		assert(Result::SUCCESS == result);

		result = CreateIocp();
		assert(Result::SUCCESS == result);

		auto bresult = CreateMessagePool();
		assert(true == bresult);

		bresult = BindListenSocketIocp();
		assert(true == bresult);

		bresult = CreateSessions();
		assert(true == bresult);

		bresult = CreateWorkerThread();
		assert(true == bresult);

		log_->Write(LogType::L_INFO, "Server started");
		return true;
	}

	void IocpServer::End()
	{
		if (INVALID_HANDLE_VALUE != worker_iocp_)
		{
			is_worker_thread_running_ = false;
			CloseHandle(worker_iocp_);

			for (size_t i = 0; i < worker_thread_vec_.size(); ++i)
			{
				if (worker_thread_vec_[i].get()->joinable())
				{
					worker_thread_vec_[i].get()->join();
				}
			}
		}

		if (INVALID_HANDLE_VALUE != message_iocp_)
		{
			CloseHandle(message_iocp_);
		}

		if (INVALID_SOCKET != listen_socket_)
		{
			closesocket(listen_socket_);
			listen_socket_ = INVALID_SOCKET;
		}
		WSACleanup();
		DestroySessions();

		log_->Write(LogType::L_INFO, "Server ended");
	}	

	void IocpServer::SendPacket(const int session_index, const void* packet, const short packet_size)
	{
		auto session = GetSession(session_index);
		if (nullptr == session)
		{
			log_->Write(LogType::L_ERROR, "%s | GetSession() failure", __FUNCTION__);
			return;
		}

		char* send_buf_reserved = nullptr;
		auto result = session->ReserveSendPacketBuffer(&send_buf_reserved, packet_size);
		if (Result::RESERVED_BUFFER_NOT_CONNECTED == result)
		{
			log_->Write(LogType::L_ERROR, "%s | Not connected failure", __FUNCTION__);
			if (session->CloseCompletely(true))
			{
				HandleSessionCloseException(session);
			}
			return;
		}
		// 받을 수 있는 최대 패킷을 초과함(비정상적 패킷)
		else if (Result::RESERVED_BUFFER_EMPTY == result)
		{
			log_->Write(LogType::L_ERROR, "%s | RingSendBuffer.ForwardSendPos() failure", __FUNCTION__);
			if (session->CloseCompletely(true))
			{
				HandleSessionCloseException(session);
			}
			return;
		}
		memcpy(send_buf_reserved, packet, packet_size);

		if (false == session->PostSend(packet_size, send_buf_reserved))
		{
			if (session->CloseCompletely(true))
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
			log_->Write(LogType::L_ERROR, "%s | WSAStartup() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_WSASTARTUP;
		}

		listen_socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == listen_socket_)
		{
			log_->Write(LogType::L_ERROR, "%s | WSASocket() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_CREATE_LISTENSOCKET;
		}

		SOCKADDR_IN addr_in;
		memset(&addr_in, 0, sizeof SOCKADDR_IN);
		addr_in.sin_family = AF_INET;
		addr_in.sin_port = htons(server_config_->port_);
		addr_in.sin_addr.s_addr = htonl(INADDR_ANY);

		if (SOCKET_ERROR == bind(listen_socket_, (sockaddr*)&addr_in, sizeof addr_in))
		{
			log_->Write(LogType::L_ERROR, "%s | bind() failure[%d]", __FUNCTION__, GetLastError());
			return Result::FAIL_BIND_LISTENSOCKET;
		}

		if (SOCKET_ERROR == listen(listen_socket_, server_config_->back_log_count_))
		{
			log_->Write(LogType::L_ERROR, "%s | listen() failure[%d]", __FUNCTION__, GetLastError());
			return Result::FAIL_LISTEN_LISTENSOCKET;
		}
		return Result::SUCCESS;
	}

	Result IocpServer::CreateIocp()
	{
		worker_iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
		if (INVALID_HANDLE_VALUE == worker_iocp_)
		{
			log_->Write(LogType::L_ERROR, "%s | CreateIoCompletionPort() failure[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_CREATE_WORKER_IOCP;
		}

		message_iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
		if (INVALID_HANDLE_VALUE == message_iocp_)
		{
			log_->Write(LogType::L_ERROR, "%s | CreateIoCompletionPort() failure[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_CREATE_LOGIC_IOCP;
		}
		return Result::SUCCESS;
	}

	bool IocpServer::CreateMessagePool()
	{
		unique_message_pool_ = make_unique<MessagePool>(server_config_->max_message_pool_count_, server_config_->extra_message_pool_count_);
		unique_message_pool_->SetLog(log_);
		if (!unique_message_pool_->CheckCounts())
		{
			return false;
		}
		return true;
	}

	bool IocpServer::BindListenSocketIocp()
	{
		auto iocp = CreateIoCompletionPort(reinterpret_cast<HANDLE>(listen_socket_), worker_iocp_, 0, 0);
		if (INVALID_HANDLE_VALUE == iocp || worker_iocp_ != iocp)
		{
			log_->Write(LogType::L_ERROR, "%s | CreateIoCompletionPort() failure: error[%d]", __FUNCTION__, WSAGetLastError());
			return false;
		}
		return true;
	}

	bool IocpServer::CreateSessions()
	{
		SessionConfig config;
		config.max_recv_buffer_size_ = server_config_->session_max_recv_buffer_size_;
		config.max_send_buffer_size_ = server_config_->session_max_send_buffer_size_;
		config.max_packet_size_	 = server_config_->max_packet_size_;

		for (int i = 0; i < server_config_->max_session_count_; ++i)
		{
			auto session = new Session();
			session->Init(listen_socket_, i, &config, log_);
			session_vec_.push_back(session);
		}
		return true;
	}

	void IocpServer::DestroySessions()
	{
		for (int i = 0; i < server_config_->max_session_count_; ++i)
		{
			delete session_vec_[i];
		}
	}

	Session* IocpServer::GetSession(const int connection_index)
	{
		if (connection_index < 0 || connection_index >= server_config_->max_session_count_)
		{
			return nullptr;
		}
		return session_vec_[connection_index];
	}

	bool IocpServer::CreateWorkerThread()
	{
		if (server_config_->worker_thread_count_ == -1)
		{
			return false;
		}

		for (int i = 0; i < server_config_->worker_thread_count_; ++i)
		{
			worker_thread_vec_.push_back(make_unique<thread>([&]() {WorkerThread(); }));
		}
		return true;
	}

	void IocpServer::WorkerThread()
	{
		while (is_worker_thread_running_)
		{
			DWORD bytes = 0;
			OverlappedEx* overlapped_ex = nullptr;
			Session* session = nullptr;

			auto result = GetQueuedCompletionStatus(
				worker_iocp_,	&bytes,
				reinterpret_cast<PULONG_PTR>(&session),
				reinterpret_cast<OVERLAPPED**>(&overlapped_ex),
				INFINITE
			);

			
			if (!result || (0 == bytes && IoMode::ACCEPT != overlapped_ex->mode_))
			{
				log_->Write(LogType::L_INFO, "%s | GQCS result = %d bytes=%d", __FUNCTION__, result, bytes);
				HandleWorkerThreadException(session, overlapped_ex);
				continue;
			}

			if (nullptr == overlapped_ex)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					log_->Write(LogType::L_ERROR, "%s | GetQueuedCompletionStatus() failure[%d]", __FUNCTION__, WSAGetLastError());
					continue;
				}
				log_->Write(LogType::L_ERROR, "%s | overlapped_ex is nullptr[%d] and WSA_IO_PENDING", __FUNCTION__, WSAGetLastError());
				continue;
			}

			switch (overlapped_ex->mode_)
			{
				case IoMode::ACCEPT:
					DoAccept(overlapped_ex);
					break;
				case IoMode::RECV:
					DoRecv(overlapped_ex, bytes);
					break;
				case IoMode::SEND:
					DoSend(overlapped_ex, bytes);
					break;
			}
		}
	}

	bool IocpServer::ProcessMessageIOCP(OUT char& msg_type, OUT int& session_index, OUT char** buf, OUT short& copy_size, int wait_mill_sec)
	{
		Message* msg = nullptr;
		Session* session = nullptr;
		DWORD bytes = 0;
		if (0 == wait_mill_sec)
		{
			wait_mill_sec = INFINITE;
		}

		auto result = GetQueuedCompletionStatus(
			message_iocp_, &bytes,
			reinterpret_cast<PULONG_PTR>(&session),
			reinterpret_cast<OVERLAPPED**>(&msg),
			wait_mill_sec
		);
		if (result == false)
		{
			this_thread::sleep_for(chrono::milliseconds(1));
			return false;
		}

		switch (msg->type_)
		{
		case MessageType::CONNECTION:
			DoPostConnection(session, msg, msg_type, session_index);
			break;

		case MessageType::CLOSE:
			DoPostClose(session, msg, msg_type, session_index);
			break;
		case MessageType::ONRECV:
			DoPostRecvPacket(session, msg, msg_type, session_index, buf, copy_size, bytes);
			unique_message_pool_.get()->DeallocateMsg(msg);
			break;
		}
		return true;
	}

	Result IocpServer::PostMessageIOCP(Session* session, Message* msg, const DWORD packet_size)
	{
		if (INVALID_HANDLE_VALUE == message_iocp_ || nullptr == msg )
		{
			log_->Write(LogType::L_ERROR, "%s | PostMessageIOCP() failure", __FUNCTION__);
			return Result::FAIL_MESSAGE_NULL;
		}

		auto result = PostQueuedCompletionStatus(
			message_iocp_, packet_size,
			reinterpret_cast<ULONG_PTR>(session),
			reinterpret_cast<OVERLAPPED*>(msg)
		);

		if (!result)
		{
			log_->Write(LogType::L_ERROR, "%s | PostQueuedCompletionStatus() failure[%d]", __FUNCTION__, WSAGetLastError());
			return Result::FAIL_PQCS;
		}
		return Result::SUCCESS;
	}

	void IocpServer::HandleWorkerThreadException(Session* session, const OverlappedEx* overlapped_ex)
	{
		if (nullptr == overlapped_ex)
		{
			log_->Write(LogType::L_ERROR, "%s | overlapped_ex is nullptr", __FUNCTION__);
			return;
		}

		switch (overlapped_ex->mode_)
		{
		case IoMode::ACCEPT:
			session->DecrementAcceptIoCount();
			break;
		case IoMode::RECV:
			session->DecrementRecvIoCount();
			break;
		case IoMode::SEND:
			session->DecrementSendIoCount();
			break;
		}

		if (session->CloseCompletely())
		{
			HandleSessionCloseException(session);
		}
		return;
	}

	void IocpServer::HandleSessionCloseException(Session* session)
	{
		if (nullptr == session)
		{
			log_->Write(LogType::L_ERROR, "%s | session is nullptr", __FUNCTION__);
			return;
		}

		if (PostMessageIOCP(session, session->GetCloseMsg()) != Result::SUCCESS)
		{
			session->ResetSession();
		}
	}

	void IocpServer::DoAccept(const OverlappedEx* overlapped_ex)
	{
		auto session = GetSession(overlapped_ex->session_index_);
		if (nullptr == session)
		{
			log_->Write(LogType::L_ERROR, "%s | session is nullptr", __FUNCTION__);
			return;
		}
		session->DecrementAcceptIoCount();

		if (false == session->SetAddressInfo())
		{
			log_->Write(LogType::L_ERROR, "%s | GetAcceptExSockaddrs() failure[%d]", __FUNCTION__, WSAGetLastError());
			if (session->CloseCompletely(true))
			{
				HandleSessionCloseException(session);
			}
			return;
		}

		if (!session->BindIocp(worker_iocp_))
		{
			if (session->CloseCompletely(true))
			{
				HandleSessionCloseException(session);
			}
			return;
		}
		session->SetStateConnected();

		auto result = session->PostRecv(0);
		if (Result::SUCCESS != result)
		{
			log_->Write(LogType::L_ERROR, "%s | PostRecv() failure[%d]", __FUNCTION__, WSAGetLastError());
			HandleSessionCloseException(session);
			return;
		}

		if (PostMessageIOCP(session, session->GetConnectionMsg()) != Result::SUCCESS)
		{
			session->Disconnect();
			session->ResetSession();
			return;
		}
	}

	void IocpServer::DoRecv(OverlappedEx* overlapped_ex, const DWORD size)
	{
		Session* session = GetSession(overlapped_ex->session_index_);
		if (nullptr == session)
		{
			log_->Write(LogType::L_ERROR, "%s | session is nullptr", __FUNCTION__);
			return;
		}

		session->DecrementRecvIoCount();
		//overlapped_ex->remain_ += size; 
		//overlapped_ex->remain_ = size; 

		//auto remain = overlapped_ex->remain_;
		//auto forwardLength = overlapped_ex->remain_;

		auto remain = size;
		log_->Write(LogType::L_INFO, "%s | remain was %d", __FUNCTION__, remain);
		auto forward_length = size;
		auto buf = overlapped_ex->wsabuf_.buf;

		// remain은 0이 되어야 한다
		ForwardPacket(session, remain, buf);
		if (0 == remain)
		{
			log_->Write(LogType::L_INFO, "%s | remain is zero", __FUNCTION__);
		}

		//if (session->PostRecv(forward_length, remain) != Result::SUCCESS)
		if (session->PostRecv(forward_length) != Result::SUCCESS)
		{
			if (session->CloseCompletely(true))
			{
				HandleSessionCloseException(session);
			}
		}
	}

	void IocpServer::ForwardPacket(Session* session, DWORD& remain, char* buf)
	{
		short packet_size = 0;

		while (true)
		{
			if (remain < kPacketHeaderLength)
			{
				break;
			}
			memcpy(&packet_size, buf, kPacketSizeLength);
			if (packet_size <= 0 || packet_size > session->GetRecvBufferSize())
			{
				log_->Write(LogType::L_ERROR, "%s | Wrong packet is received", __FUNCTION__);
				if (session->CloseCompletely(true))
				{
					HandleSessionCloseException(session);
				}
				return;
			}

			if (remain >= static_cast<DWORD>(packet_size))
			{
				auto msg = unique_message_pool_->AllocateMsg();
				if (nullptr == msg)
				{
					log_->Write(LogType::L_ERROR, "%s | Message is empty", __FUNCTION__);
					return;
				}

				msg->SetMessagae(MessageType::ONRECV, buf);
				if (PostMessageIOCP(session, msg, packet_size) != Result::SUCCESS)
				{
					unique_message_pool_->DeallocateMsg(msg);
					return;
				}
				remain -= packet_size;
				buf += packet_size;
			}
			else
			{
				break;
			}
		}
	}

	void IocpServer::DoSend(OverlappedEx* overlapped_ex, const DWORD size)
	{
		Session* session = GetSession(overlapped_ex->session_index_);
		if (nullptr == session)
		{
			log_->Write(LogType::L_ERROR, "%s | session is nullptr", __FUNCTION__);
			return;
		}

		session->DecrementSendIoCount();
		overlapped_ex->remain_ += size;

		// 모든 메시지를 전송하지 못했을 때
		if (static_cast<DWORD>(overlapped_ex->total_bytes_) > overlapped_ex->remain_)
		{
			log_->Write(LogType::L_ERROR, "%s | 전송 진행 중", __FUNCTION__);
			session->IncrementSendIoCount();
			overlapped_ex->wsabuf_.buf += size;
			overlapped_ex->wsabuf_.len -= size;
			memset(&overlapped_ex->overlapped_, 0, sizeof OVERLAPPED);

			DWORD bytes = 0;
			auto result = WSASend(
				session->GetClientSocket(),
				&overlapped_ex->wsabuf_, 1,
				&bytes, 0, 
				&overlapped_ex->overlapped_, nullptr
			);

			if (SOCKET_ERROR == result && WSA_IO_PENDING != WSAGetLastError())
			{
				session->DecrementSendIoCount();
				log_->Write(LogType::L_ERROR, "%s | WSASend() failure[%d]", __FUNCTION__, WSAGetLastError());
				if (session->CloseCompletely(true))
				{
					HandleSessionCloseException(session);
				}
				return;
			}
		}

		// 모든 메시지 전송
		else if (static_cast<DWORD>(overlapped_ex->total_bytes_) == overlapped_ex->remain_)
		{
			session->SetSendAvaliable();
		}
	}

	void IocpServer::DoPostConnection(Session* session, const Message* msg, OUT char& msg_type, OUT int& session_index)
	{
		if (false == session->IsConnected())
		{
			log_->Write(LogType::L_ERROR, "%s | Not connected", __FUNCTION__);
			return;
		}

		msg_type = static_cast<char>(msg->type_);
		session_index = session->GetIndex();
	}

	void IocpServer::DoPostClose(Session* session, const Message* msg, OUT char& msg_type, OUT int& session_index)
	{
		msg_type = static_cast<char>(msg->type_);
		session_index = session->GetIndex();
		session->ResetSession();
	}

	void IocpServer::DoPostRecvPacket(Session* session, const Message* msg, OUT char& msg_type, OUT int& session_index, OUT char** buf, OUT short& copy_size, const DWORD size)
	{
		if (nullptr == msg->contents_)
		{
			log_->Write(LogType::L_ERROR, "%s | Message contents is nullptre", __FUNCTION__);
			return;
		}

		msg_type = static_cast<char>(msg->type_);
		session_index = session->GetIndex();
		copy_size = static_cast<short>(size);
		*buf = msg->contents_;
	}
}