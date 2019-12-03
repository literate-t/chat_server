#pragma once
#include "stdafx.h"

namespace ServerLibrary
{
	class Session
	{
		using LockGuard = Lock::LockGuard;

	public:
		Session() = default;
		~Session() {}

		Message* GetConnectionMsg() { return &ConnectionMsg; }
		Message* GetCloseMsg() { return &CloseMsg; }

		void Init(const SOCKET listenSocket, const int index, const SessionConfig* config, ILog* log);
		void SetLog(ILog* log);

		bool CloseCompletely();
		void Disconnect(bool forced = false);
		Result ResetSession();
		bool BindIocp(const HANDLE WorkerIocp);
		Result PostRecv(const int forwardLength, const DWORD remainByte);
		bool PostSend(const int sendSize, char* buffer);
		Result ReserveSendPacketBuffer(OUT char** buf, const int sendSize);

		SOCKET GetClientSocket() { return ClientSocket; }
		void SetIp(const char* ip) { memcpy(Ip, ip, kMaxIpLength); }
		int GetIndex() { return Index; }

		void IncrementAcceptIoCount() { ++AcceptIoCount; }
		void DecrementAcceptIoCount() { AcceptIoCount = AcceptIoCount ? --AcceptIoCount : 0; }
		void IncrementRecvIoCount() { ++RecvIoCount; }
		void DecrementRecvIoCount() { RecvIoCount = RecvIoCount ? --RecvIoCount : 0; }
		void IncrementSendIoCount() { ++SendIoCount; }
		void DecrementSendIoCount() { SendIoCount = SendIoCount ? --SendIoCount : 0; }
		short GetRecvIoCount() { return RecvIoCount.load(); }

		bool IsConnected() { return Connected; }
		void SetStateConnected() { InterlockedExchange(reinterpret_cast<long*>(&Connected), TRUE); }
		void SetStateDisconnected() { InterlockedExchange(reinterpret_cast<long*>(&Connected), FALSE); }

		int GetRecvBufferSize() { return RingRecvBuffer.GetBufferSize(); }
		char* GetRecvBufferBegin() { return RingRecvBuffer.GetBegin();}
		void ReleaseRecvBuffer(const int size) { RingRecvBuffer.ReleaseBuffer(size); }

		bool SetAddressInfo();
		void ReleaseSendBuffer(const int size) { RingSendBuffer.ReleaseBuffer(size); }
		void SetSendAvaliable() { InterlockedExchange(reinterpret_cast<long*>(&Sendable), TRUE); }
		

	private:
		void Init();
		Result AcceptExSocket();		

	private:
		int Index = -1;
		SOCKET ClientSocket = INVALID_SOCKET;
		SOCKET ListenSocket = INVALID_SOCKET;

		OverlappedEx*	RecvOverlappedEx = nullptr;
		OverlappedEx*	SendOverlappedEx = nullptr;

		RingBuffer		RingRecvBuffer;
		RingBuffer		RingSendBuffer;
		int				MaxPacketSize = -1;

		char			AddrBuf[kMaxAddrLength] = { 0 };

		BOOL Connected	= FALSE;
		BOOL Sendable	= FALSE;

		int RecvBufSize = -1;
		int SendBufSize = -1;

		char Ip[kMaxIpLength] = { 0 };

		atomic<short> AcceptIoCount = 0;
		atomic<short> RecvIoCount	= 0;
		atomic<short> SendIoCount	= 0;

		Message ConnectionMsg;
		Message CloseMsg;

		ILog*	Log;
		Lock	Cs;
	};
}