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
	// 내부 버퍼 읽어서 반환(?)
	char* GetBuffer(const int req_read_size, OUT int& res_read_size);

private:
	char*	buffer_;
	char*	begin_;
	char*	end_;
	char*	write_mark_;// CurrentMark
	char*	read_mark_; // GettedBufferMark(?)
	char*	last_mark_;

	int		buffer_size_;	// 버퍼 총 크기
	int		write_size_;	// 사용 중인 크기(nUsedBufferSize)
	unsigned int total_data_size_;// 처리된 데이터 총량(AllUserBufSize)

	Lock cs_;
};