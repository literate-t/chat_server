#pragma once
#include "stdafx.h"

namespace ServerLibrary
{
	const int kMaxIpLength = 20;
	const int kMaxAddrLength = 64;

	struct ServerConfig
	{
		unsigned short Port						=  0;
		int BackLogCount						= -1;
		int WorkerThreadCount					= -1;
		int SessionMaxRecvBufferSize			= -1; // 받기용 버퍼의 최대 크기(데이터를 받으면 여기에 저장되고, 데이터의 위치가 애플리케이션에 전달되므로 넉넉하게 커야 한다)
		int SessionMaxSendBufferSize			= -1; // 보내기용 버퍼의 최대 크기
		int MaxPacketSize						= -1;
		int MaxSessionCount						= -1;
		int MaxMessagePoolCount					= -1;
		int ExtraMessagePoolCount				= -1;
		int PerformancePacketMillisecondsTime	= -1;

		int MaxLobbyCount						= -1;
		int MaxLobbyUserCount					= -1;
		int MaxRoomCount						= -1;
		int MaxRoomUserCount					= -1;
	};	
	
	struct SessionConfig
	{
		int MaxRecvBufferSize	= -1;
		int MaxSendBufferSize	= -1;
		int MaxPacketSize		= -1;
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
		int				SessionIndex = 0;

		OverlappedEx(int sessionIndex)
		{
			memset(this, 0, sizeof OverlappedEx);
			SessionIndex = sessionIndex;
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

	enum class Result : short
	{
		SUCCESS = 0,

		FAIL_MAKE_DIRECTORY_LOG,
		FAIL_MAKE_DIRECTORY_DUMP,
		FAIL_WORKTHREAD_INFO,

		FAIL_SERVER_INFO_PORT,
		FAIL_SERVER_INFO_MAX_RECVBUF_SIZE,
		FAIL_SERVER_INFO_MAX_SENDBUF_SIZE,
		FAIL_SERVER_INFO_MAX_RECV_CONNECTIONBUF_SIZE,
		FAIL_SERVER_INFO_MAX_SEND_CONNECTIONBUF_SIZE,
		FAIL_SERVER_INFO_MAX_PACKET_SIZE,
		FAIL_SERVER_INFO_MAX_CONNECTION_COUNT,
		FAIL_SERVER_INFO_MAX_MESSAGEPOOL_COUNT,
		FAIL_SERVER_INFO_EXTRA_MESSAGEPOOL_COUNT,
		FAIL_SERVER_INFO_PERFORMANCE_PACKET_MILLISECONDS_TIME,
		FAIL_SERVER_INFO_POSTMESSAGES_THREAD_COUNT,

		FAIL_WSASTARTUP,
		FAIL_CREATE_LISTENSOCKET,
		FAIL_BIND_LISTENSOCKET,		
		FAIL_LISTEN_LISTENSOCKET,

		FAIL_CREATE_WORKER_IOCP,
		FAIL_CREATE_LOGIC_IOCP,

		FAIL_CREATE_MESSAGE_MANAGER,
		FAIL_BIND_IOCP,

		FAIL_CREATE_CONNECTION,
		FAIL_CREATE_PERFORMANCE,
		FAIL_CREATE_WORKERTHREAD,
		
		FAIL_WSASOCKET,
		FAIL_ACCEPTEX,
		FAIL_MESSAGE_NULL,
		FAIL_PQCS,

		POSTRECV_NULL_OBJ,
		POSTRECV_NULL_WSABUF,
		POSTRECV_NULL_SOCKET_ERROR,

		RESERVED_BUFFER_NOT_CONNECTED,
		RESERVED_BUFFER_EMPTY,

		FUNCTION_RESULT_END
	};
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