#pragma once
//#include "Lock.h"

namespace ServerLibrary
{
	class RingBuffer : public Lock
	{
		using LockGuard = Lock::LockGuard;

	public:
		RingBuffer() = default;
		~RingBuffer();

		bool Create(int buffer_size);
		bool Init();
		inline int GetBufferSize() { return BufferSize; }

		inline char* GetBegin() { return Begin; }
		inline char* GetWriteMark() { return WritePos; }
		inline char* GetEndMark() { return End; }

		char* ForwardSendPos(const int forward_length);
		char* ForwardRecvPos(const int forward_length);

		void ReleaseBuffer(int release_size);
		int GetUsedBufferSize() { return WriteSize; }
		int GetTotalUsedBufferSize() { return TotalDataSize; }
		// �۽��� ���� �о����(���� �ʿ�)
		char* GetBuffer(const int req_read_size, OUT int& res_read_size);

	private:
		char* Buffer		= nullptr;
		char* Begin			= nullptr;
		char* End			= nullptr;
		char* WritePos		= nullptr; // CurrentMark
		char* ReadPos		= nullptr; // GettedBufferMark(?)
		char* LastPos		= nullptr;

		int	BufferSize		= 0;	// ���� �� ũ��
		int	WriteSize		= 0;	// ��� ���� ũ��(nUsedBufferSize)
		unsigned int TotalDataSize = 0;// ó���� ������ �ѷ�(AllUserBufSize)

		Lock cs_;
	};
}