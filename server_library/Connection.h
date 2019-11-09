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

		bool	RecvPost(char* next, DWORD remain); // remain�� ��Ȯ�� �ǹ̸� �ľ��� ��
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
		// overlapped i/o ���� �۾� ���� ����
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

		//// i/o �۾� ��û Ƚ��
		// �۾��� ��û�ϸ� 1 ����, �۾��� �Ϸ�Ǹ� 1 ����
		// �� ���� �۾��� ��û�� ���¸� ��������
		// ��ü ������ ������� �� ù ��° ��û�� ��ҵǾ�
		// GQCS�� ���� ����� ��ȯ�ȴ�.
		// ������ ������ ��ü�� �ʱ�ȭ�ϰ� AcceptEx�� ���ο� Ŭ���̾�Ʈ�� �޾�����
		// �ռ� ��û�� �۾� �� �� ���� ���� �־�
		// �ٷ� GQCS�� ���� ����� ��ȯ�Ǿ� ������ �ٽ� �ʱ�ȭ�ȴ�.
		// �̷� ������ �۾� ��û Ƚ���� ������ 0�� ���� �� ������ �����Ѵ�.
		// (pool ������� ��ü�� �����ϱ� ������, �޸𸮸� ���� �Ҵ����� �ʾ�
		// ����� ������ ��)
		DWORD	recv_io_cnt_;
		DWORD	send_io_cnt_;
		DWORD	accept_io_cnt_;

		Lock	cs_;
	};
}