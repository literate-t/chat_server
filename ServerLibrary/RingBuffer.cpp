#include "stdafx.h"

namespace ServerLibrary
{
	const int kMaxPacketSize = 1024;
	RingBuffer::~RingBuffer()
	{
		delete[] Buffer;
	}

	bool RingBuffer::Init()
	{
		LockGuard lock(cs_);
		memset(Begin, 0, BufferSize);
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
	char* RingBuffer::ForwardSendPos(const int forwardLength)
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

	// 수신									
	char* RingBuffer::ForwardRecvPos(const int forwardLength)
	{
		LockGuard lock(cs_);
		LastPos = WritePos;
		WritePos += forwardLength;
		if (static_cast<int>(End - WritePos) < kMaxPacketSize)
		{
			memcpy(Begin, LastPos, forwardLength);
			WritePos = Begin + forwardLength;
		}
		
		WriteSize		+= forwardLength;
		TotalDataSize	+= forwardLength;

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

		// LastPos는 순회했다면 순회하기 전 위치이며 안 했다면 End와 같다
		if (LastPos == ReadPos)
		{
			ReadPos = Begin;
			LastPos = End;
		}

		// origin : (WriteSize > reqReadSize
		if (WriteSize >= reqReadSize)
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