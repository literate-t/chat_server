#pragma once

namespace ServerLibrary
{
	class RingBuffer
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

	private:
		char* Buffer		= nullptr;
		char* Begin			= nullptr;
		char* End			= nullptr;
		char* WritePos		= nullptr; // CurrentMark
		char* ReadPos		= nullptr; // GettedBufferMark(?)
		char* LastPos		= nullptr;

		int	BufferSize		= 0;	// 버퍼 총 크기
		int	WriteSize		= 0;	// 사용 중인 크기(nUsedBufferSize)
		unsigned int TotalDataSize = 0;// 처리된 데이터 총량(AllUserBufSize)

		Lock cs_;
	};
}