#pragma once

#define MAX_RINGBUFSIZE		1024 * 50
#define MAX_VBUFSIZE		1024 * 40
#define MAX_PBUFSIZE		4096			//PacketPool에서 버퍼 한개당 size
#define MAX_QUEUESIZE		10000

#define MAX_RECVBUFCNT		100				
#define MAX_SENDBUFCNT		100				

#define PACKET_LENGTH_BYTE	4
#define PACKET_TYPE_BYTE	2

#define MAX_IP_LENGTH		20
#define MAX_PROCESS_THREAD	1
#define MAX_WORKER_THREAD	10

enum IoMode
{
	ACCEPT,
	RECV,
	SEND,
	CLOSE,
	PROCESS_PACKET,
	SYSTEM
};

struct InitConfig
{
	int		index_ = -1;
	SOCKET	sock_listener_ = INVALID_SOCKET;
	// ring buffer size = buf_cnt * buf_size
	int		recv_buf_cnt_ = 0;
	int		send_buf_cnt_ = 0;
	int		recv_buf_size_ = 0;
	int		send_buf_size_ = 0;

	int		process_packet_cnt_ = 0;
	int		server_port_ = 0;
	int		worker_thread_cnt_ = 0;
	int		process_thread_cnt_ = 0;
};

struct OverlappedEx
{
	WSAOVERLAPPED	overlapped_;
	WSABUF			wsabuf_;
	int				total_bytes_; // 총 송수신된 양
	DWORD			still_;		  // 현재까지 받은 패킷의 바이트 수(remain)
	char*			msg_;
	IoMode			iomode_;
	void*			connection_;
	explicit OverlappedEx(void* conn)
	{
		memset(this, 0, sizeof OverlappedEx);
		connection_ = conn;
	}
};