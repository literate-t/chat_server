#pragma once

namespace library
{
	class Connection : public Lock
	{
		using LockGurad = Lock::LockGuard;
	public:
		Connection();
		~Connection();

	public:
		void	Init();
		bool	CreateConnection(InitConfig& config);
		bool	CloseConnection(bool is_forced = false);
		bool	BindIocp(HANDLE iocp);
		bool	BindAcceptEx();

		bool	RecvPost(char* next, DWORD remain); // remain의 정확한 의미를 파악할 것
		bool	SendPost();

		void	SetSocket(SOCKET socket) { socket_ = socket; }
		SOCKET	GetSocket()				 { return socket_; }
		char*	PrepareSendPacket(int len);
		bool	ReleaseRecvPacket();
		bool	ReleaseSendPacket(OverlappedEx* send_overlappedex = nullptr);
		void	SetConnection(const char* ip);

		char* GetIp()			{ return ip_; }
		int GetIndex()			{ return index_; }
		int GetRecvBufSize()	{ return recv_buf_size_; }
		int GetSendBufSize()	{ return send_buf_size_; }

		int GetRecvIoCount()	{ return recv_io_cnt_; }
		int GetSendIoCount()	{ return send_io_cnt_; }
		int GetAceeptIoCount()	{ return accept_io_cnt_; }

		int IncrementRecvIoCount()		{ return InterlockedIncrement(&recv_io_cnt_); }
		int IncrementSendIoCount()		{ return InterlockedIncrement(&send_io_cnt_); }
		int IncrementAcceptIoCount()	{ return InterlockedIncrement(&accept_io_cnt_); }

		int DecrementRecvIoCount();
		int DecrementSendIoCount();
		int DecrementAcceptIoCount();

	public:
		OverlappedEx	recv_overlappedex_;
		OverlappedEx	send_overlappedex_;

		RingBuffer		recv_ring_buffer_;
		RingBuffer		send_ring_buffer_;

		char	address_[1024];
		BOOL	is_closed_;
		BOOL	is_connected_;
		// overlapped i/o 전송 작업 진행 여부
		BOOL	is_sent_;

	private:
		SOCKET	socket_;
		SOCKET	socket_listener_;
		Logger  logger_;

		int		recv_buf_size_;
		int		send_buf_size_;
		char	ip_[MAX_IP_LENGTH];
		int		index_;

		HANDLE	iocp_;

		//// i/o 작업 요청 횟수
		// 작업을 요청하면 1 증가, 작업이 완료되면 1 감소
		// 세 개의 작업을 요청한 상태를 가정하자
		// 객체 연결이 종료됐을 때 첫 번째 요청이 취소되어
		// GQCS를 통해 결과가 반환된다.
		// 연결이 해제된 객체를 초기화하고 AcceptEx로 새로운 클라이언트를 받았지만
		// 앞서 요청한 작업 중 두 개가 남아 있어
		// 바로 GQCS를 통해 결과과 반환되어 연결이 다시 초기화된다.
		// 이런 이유로 작업 요청 횟수를 추적해 0이 됐을 때 연결을 해제한다.
		// (pool 방식으로 객체를 관리하기 때문에, 메모리를 새로 할당하지 않아
		// 생기는 문제인 듯)
		DWORD	recv_io_cnt_;
		DWORD	send_io_cnt_;
		DWORD	accept_io_cnt_;

		Lock	cs_;
	};
}