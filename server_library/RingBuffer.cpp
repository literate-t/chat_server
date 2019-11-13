#include "stdafx.h"

namespace library
{
	RingBuffer::~RingBuffer()
	{
		delete[] Buffer;
	}

	bool RingBuffer::Init()
	{
		LockGuard lock(cs_);
		WriteSize = 0;
		WritePos = Begin;
		ReadPos = Begin;
		LastPos = End;
		TotalDataSize = 0;
		return true;
	}

	bool RingBuffer::Create(int bufferSize)
	{
		{
			LockGuard lock(cs_);
			if (bufferSize <= 0)
			{
				return false;
			}
			BufferSize = bufferSize;

			if (nullptr != Buffer)
			{
				delete[] Buffer;
				Buffer = nullptr;
			}
			Buffer = new char[BufferSize];

			Begin = Buffer;
			End = Begin + BufferSize - 1;
		}

		Init();
		return true;
	}

	// 송신
	char* RingBuffer::ForwardMark(const int forwardLength)
	{
		LockGuard lock(cs_);
		char* prev = nullptr;
		if (WriteSize + forwardLength > BufferSize)
		{
			return nullptr;
		}

		if ((End - WritePos) >= forwardLength)
		{
			prev = WritePos;
			WritePos += forwardLength;
		}

		else
		{
			LastPos = WritePos;
			WritePos = Begin + forwardLength;
			prev = Begin;
		}

		WriteSize += forwardLength;
		TotalDataSize += forwardLength;

		return prev;
	}

	// 수신									현재 받은 데이터, 다음에 받을 데이터 길이, 현재까지 받은 패킷의 길이
	char* RingBuffer::ForwardMark(const int forwardLength, const int nextLength, const DWORD remain)
	{
		LockGuard lock(cs_);
		if (WriteSize + forwardLength + nextLength > BufferSize)
		{
			return nullptr;
		}

		if (static_cast<int>(End - WritePos) > forwardLength + nextLength)
		{
			WritePos		+= forwardLength;
			WriteSize		+= forwardLength;
			TotalDataSize	+= forwardLength;
		}

		else
		{
			LastPos = WritePos;
			////이건 진짜 아무리 생각해도 이상하네. 아예 산술적으로 맞지가 않음
			//////memcpy(begin_, write_mark_ - (sofar_length - forward_length), sofar_length);
			////memcpy(begin_, write_mark_ - sofar_length, sofar_length);
			////write_mark_ = begin_ + sofar_length;
			//memcpy(begin_, write_mark_, forward_length);
			//write_mark_ = begin_ + forward_length;
			memcpy(Begin, WritePos - (remain - forwardLength), remain);
			WritePos		= Begin + remain;
			WriteSize		+= remain;
			TotalDataSize	+= remain;
		}

		return WritePos;
	}

	void RingBuffer::ReleaseBuffer(int releaseSize)
	{
		LockGuard lock(cs_);
		WriteSize = (releaseSize > WriteSize) ? 0 : (WriteSize - releaseSize);
	}

	
	char* RingBuffer::GetBuffer(const int reqReadSize, OUT int& resReadSize)
	{
		LockGuard lock(cs_);
		char* buffer = nullptr;

		// LastPos는 순회했다면 순회하기 전 위치이며 안 했다면 end_와 같다
		if (LastPos == ReadPos)
		{
			ReadPos = Begin;
			LastPos = End;
		}

		if (WriteSize > reqReadSize)
		{
			if (LastPos - ReadPos >= reqReadSize)
			{
				resReadSize = reqReadSize;
			}
			else
			{
				resReadSize = static_cast<int>(LastPos - ReadPos);
			}
			buffer = ReadPos;
			ReadPos += resReadSize;
		}
		else if (WriteSize > 0)
		{
			if (LastPos - ReadPos >= WriteSize)
			{
				resReadSize = WriteSize;
				buffer = ReadPos;
				ReadPos += WriteSize;
			}
			else
			{
				resReadSize = static_cast<int>(LastPos - ReadPos);
				buffer = ReadPos;
				ReadPos += resReadSize;
			}
		}

		return buffer;
	}
}