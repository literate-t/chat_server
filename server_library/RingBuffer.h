#pragma once

class RingBuffer : public Lock
{
	using LockGuard = Lock::LockGuard;

public:
	RingBuffer();
	~RingBuffer();

	bool Create(int buffer_size = MAX_RINGBUFSIZE);
	bool Init();
	inline int GerBufferSize()	{ return buffer_size_; }
	
	inline char* GetBegin()		{ return begin_; }
	inline char* GetWriteMark() { return write_mark_; }
	inline char* GetEndMark()	{ return end_; }

	char* ForwardMark(const int forward_length);
	char* ForwardMark(const int forward_length, const int next_length, const DWORD sofar_length);

	void ReleaseBuffer(int release_size);
	int GetUsedBufferSize()		{ return write_size_; }
	int GetTotalUsedBufferSize(){ return total_data_size_; }
	// ���� ���� �о ��ȯ(?)
	char* GetBuffer(const int req_read_size, OUT int& res_read_size);

private:
	char*	buffer_;
	char*	begin_;
	char*	end_;
	char*	write_mark_;// CurrentMark
	char*	read_mark_; // GettedBufferMark(?)
	char*	last_mark_;

	int		buffer_size_;	// ���� �� ũ��
	int		write_size_;	// ��� ���� ũ��(nUsedBufferSize)
	unsigned int total_data_size_;// ó���� ������ �ѷ�(AllUserBufSize)

	Lock cs_;
};