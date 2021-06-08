#include "stdafx.h"

namespace server_library
{
	const int kMaxPacketSize = 1024;
	RingBuffer::~RingBuffer()
	{
		delete[] buffer_;
	}

	bool RingBuffer::Init()
	{
		LockGuard lock(cs_);
		memset(begin_, 0, buffer_size_);
		write_size_ = 0;
		write_pos_ = begin_;
		read_pos_ = begin_;
		last_pos_ = end_;
		total_data_size_ = 0;
		return true;
	}

	bool RingBuffer::Create(int buffer_size)
	{
		{
			LockGuard lock(cs_);
			if (buffer_size <= 0)
			{
				return false;
			}
			buffer_size_ = buffer_size;

			if (nullptr != buffer_)
			{
				delete[] buffer_;
				buffer_ = nullptr;
			}
			buffer_ = new char[buffer_size_];

			begin_ = buffer_;
			end_ = begin_ + buffer_size_ - 1;
		}

		Init();
		return true;
	}

	// 수신									
	char* RingBuffer::ForwardRecvPos(const int forward_length)
	{
		LockGuard lock(cs_);
		last_pos_ = write_pos_;
		write_pos_ += forward_length;
		if (static_cast<int>(end_ - write_pos_) < kMaxPacketSize)
		{
			memcpy(begin_, last_pos_, forward_length);
			write_pos_ = begin_ + forward_length;
		}
		
		write_size_			+= forward_length;
		total_data_size_	+= forward_length;

		return write_pos_;
	}

	// 송신
	char* RingBuffer::ForwardSendPos(const int forward_length)
	{
		LockGuard lock(cs_);
		char* prev = nullptr;
		//if (write_size_ + forward_length > buffer_size_)
		if (forward_length > buffer_size_)
		{
			return nullptr;
		}

		if ((end_ - write_pos_) >= forward_length)
		{
			prev = write_pos_;
			write_pos_ += forward_length;
		}

		else
		{
			last_pos_ = write_pos_;
			write_pos_ = begin_ + forward_length;
			prev = begin_;
		}

		write_size_ += forward_length;
		total_data_size_ += forward_length;

		return prev;
	}	

	void RingBuffer::ReleaseBuffer(int release_size)
	{
		LockGuard lock(cs_);
		write_size_ = (release_size > write_size_) ? 0 : (write_size_ - release_size);
	}
}