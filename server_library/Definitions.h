#pragma once

//#define MAX_RINGBUFSIZE		1024 * 50
//#define MAX_VBUFSIZE		1024 * 40
//#define MAX_PBUFSIZE		4096			//PacketPool에서 버퍼 한개당 size
//#define MAX_QUEUESIZE		10000
//
//#define MAX_RECVBUFCNT		100				
//#define MAX_SENDBUFCNT		100				
//
//#define PACKET_HEADER_BYTE  6
//#define PACKET_LENGTH_BYTE	4
//#define PACKET_TYPE_BYTE	2
//
//#define MAX_IP_LENGTH		20
//#define MAX_PROCESS_THREAD	1
#include "stdafx.h"

namespace library
{
	struct Config
	{
		unsigned short PortNumber			=  0;
		int WorkThreadCount					= -1;
		int MaxRecvOverlappedBufferSize		= -1; // 한번에 받을 수 있는 최대 크기
		int MaxSendOverlappedBufferSize		= -1; // 한번에 보낼 수 있는 최대 크기
		int ConnectionMaxRecvBufferSize		= -1; // 받기용 버퍼의 최대 크기(데이터를 받으면 여기에 저장되고, 데이터의 위치가 애플리케이션에 전달되므로 넉넉하게 커야 한다)
		int ConnectionMaxSendBufferSize		= -1; // 보내기용 버퍼의 최대 크기
		int MaxPacketSize						= -1;
		int MaxConnectionCount				= -1;
		int MaxMessagePoolCount				= -1;
		int ExtraMessagePoolCount				= -1;
		int PerformancePacketMillisecondsTime = -1;
	};

	struct ConnectionConfig
	{
		int MaxRecvBufferSize				= -1;
		int MaxSendBufferSize				= -1;
		int MaxRecvOverlappedBufferSize		= -1;
		int MaxSendOverlappedBufferSize		= -1;
	};

	enum class IoMode : char
	{
		NONE = 0,
		ACCEPT,
		RECV,
		SEND
	};

	enum class MessageType : char
	{
		NONE = 0,
		CONNECTION,
		CLOSE,
		ONRECV
	};

	struct OverlappedEx
	{
		WSAOVERLAPPED	Overlapped;
		WSABUF			Wsabuf;
		IoMode			Mode;

		int				TotalBytes; // 총 송수신된 양
		DWORD			Remain;		  // 현재까지 받은 패킷의 바이트 수(remain)
		char*			SocketMsg;
		int				ConnectionIndex = 0;

		OverlappedEx(int connectionIndex_)
		{
			memset(this, 0, sizeof OverlappedEx);
			ConnectionIndex = connectionIndex_;
		}
	};

	struct Message
	{
		MessageType Type = MessageType::NONE;
		char* Contents = nullptr;

		void Clear()
		{
			Type = MessageType::NONE;
			Contents = nullptr;
		}

		void SetMessagae(MessageType type, char* contents)
		{
			Type = type;
			Contents = contents;
		}
	};

	const int kMaxIpLength		= 20;
	const int kMaxAddrLength	= 64;
}



//struct InitConfig
//{
//	int		index_ = -1;
//	SOCKET	sock_listener_ = INVALID_SOCKET;	
//	// ring buffer size = buf_cnt * buf_size
//	int		recv_buf_cnt_ = 0;	
//	int		send_buf_cnt_ = 0;	
//	int		recv_buf_size_ = 0;	
//	int		send_buf_size_ = 0;	
//
//	int		process_packet_cnt_ = 0;
//	int		server_port_ = 0;
//	int		worker_thread_cnt_ = 0;
//	int		process_thread_cnt_ = 0;
//};



//struct PacketProcess
//{
//	IoMode iomode_;
//	WPARAM wparam_;
//	LPARAM lparam_;
//	int	   index_;
//	PacketProcess()
//	{
//		Init();
//	}
//	void Init()
//	{
//		memset(this, 0, sizeof PacketProcess);
//	}
//};