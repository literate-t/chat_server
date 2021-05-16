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
		inline int GetBufferSize() { return buffer_size_; }

		inline char* GetBegin() { return begin_; }
		inline char* GetWriteMark() { return write_pos_; }
		inline char* GetEndMark() { return End; }

		char* ForwardSendPos(const int forward_length);
		char* ForwardRecvPos(const int forward_length);

		void ReleaseBuffer(int release_size);
		int GetUsedBufferSize() { return write_size_; }
		int GetTotalUsedBufferSize() { return total_data_size_; }

	private:
		char* buffer_		= nullptr;
		char* begin_			= nullptr;
		char* end_			= nullptr;
		char* write_pos_		= nullptr; // CurrentMark
		char* read_pos_		= nullptr; // GettedBufferMark(?)
		char* last_pos_		= nullptr;

		int	buffer_size_		= 0;	// ���� �� ũ��
		int	write_size_		= 0;	// ��� ���� ũ��(nUsedBufferSize)
		unsigned int total_data_size_ = 0;// ó���� ������ �ѷ�(AllUserBufSize)

		Lock cs_;
	};
}