#include "stdafx.h"

RingBuffer::RingBuffer()
{
	buffer_			= nullptr;
	begin_			= nullptr;
	end_			= nullptr;
	write_mark_		= nullptr;
	read_mark_		= nullptr;
	last_mark_		= nullptr;
	buffer_size_	= 0;
	write_size_		= 0;
	total_data_size_= 0;
}

RingBuffer::~RingBuffer()
{
	if (nullptr != begin_) 
	{
		delete[] begin_;
	}
}

bool RingBuffer::Init()
{
	LockGuard lock(cs_);
	write_size_			= 0;
	write_mark_			= begin_;
	read_mark_			= begin_;
	last_mark_			= end_;
	total_data_size_	= 0;

	return true;
}

bool RingBuffer::Create(int buffer_size)
{
	{
		LockGuard lock(cs_);
		if (nullptr != begin_)
		{
			delete[] begin_;
			begin_ = nullptr;
		}

		begin_ = new char[buffer_size];
		if (nullptr == begin_)
		{
			return false;
		}

		end_ = begin_ + buffer_size - 1;
		buffer_size_ = buffer_size;
	}

	Init();
	return true;
}

// 송신
char* RingBuffer::ForwardMark(const int forward_length)
{
	LockGuard lock(cs_);
	char* prev = nullptr;
	if (write_size_ + forward_length > buffer_size_)
	{
		return nullptr;
	}

	if ((end_ - write_mark_) >= forward_length)
	{
		prev = write_mark_;
		write_mark_ += forward_length;
	}

	else
	{
		last_mark_ = write_mark_;
		write_mark_ = begin_ + forward_length;
		prev = begin_;
	}

	write_size_			+= forward_length;
	total_data_size_	+= forward_length;

	return prev;
}

// 수신									현재 받은 데이터, 다음에 받을 데이터 길이, 현재까지 받은 패킷의 길이
char* RingBuffer::ForwardMark(const int forward_length, const int next_length, const DWORD sofar_length)
{
	LockGuard lock(cs_);
	if (write_size_ + forward_length + next_length > buffer_size_)
	{
		return nullptr;
	}

	if (static_cast<int>(end_ - write_mark_) > forward_length + next_length)
	{
		write_mark_ += forward_length;
	}

	else
	{
		last_mark_ = write_mark_;
		//이건 진짜 아무리 생각해도 이상하네. 아예 산술적으로 맞지가 않음
		////memcpy(begin_, write_mark_ - (sofar_length - forward_length), sofar_length);
		//memcpy(begin_, write_mark_ - sofar_length, sofar_length);
		//write_mark_ = begin_ + sofar_length;
		memcpy(begin_, write_mark_, forward_length);
		write_mark_ = begin_ + forward_length;
	}
	
	write_size_ += forward_length;
	total_data_size_ += forward_length;

	return write_mark_;
}

void RingBuffer::ReleaseBuffer(int release_size)
{
	LockGuard lock(cs_);
	write_size_ -= release_size;
}

// 송신할 버퍼 읽어오기(수정 필요)
char* RingBuffer::GetBuffer(const int req_read_size, OUT int& res_read_size)
{
	LockGuard lock(cs_);

	// last_mark_는 순회했다면 순회하기 전 위치이며 안 했다면 end_와 같다
	if (last_mark_ == read_mark_)
	{
		read_mark_ = begin_;
		last_mark_ = end_;
	}
	char* buffer = read_mark_;

	if (write_size_ > req_read_size)
	{
		res_read_size = req_read_size;
		read_mark_ += res_read_size;
	}

	return buffer;
}