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

	// �۽�
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

	// ����									���� ���� ������, ������ ���� ������ ����, ������� ���� ��Ŷ�� ����
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
			////�̰� ��¥ �ƹ��� �����ص� �̻��ϳ�. �ƿ� ��������� ������ ����
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

		// LastPos�� ��ȸ�ߴٸ� ��ȸ�ϱ� �� ��ġ�̸� �� �ߴٸ� end_�� ����
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