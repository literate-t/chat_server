#include "stdafx.h"
#include "VBuffer.h"

VBuffer::VBuffer(int max_buf_size)
{
	vbuf_ = new char[max_buf_size];
	max_buf_size_ = max_buf_size;
	Init();
}

VBuffer::~VBuffer()
{
	if (nullptr != vbuf_)
	{
		delete[] vbuf_;
	}
}

void VBuffer::Init()
{
	cur_mark_		= vbuf_ + PACKET_LENGTH_BYTE;
	cur_buf_size_	= PACKET_LENGTH_BYTE;
}

void VBuffer::GetChar(OUT char& ch)
{
	ch = (unsigned char)cur_mark_[0];
	cur_mark_		+= 1;
	cur_buf_size_	+= 1;
}

void VBuffer::GetShort(OUT short& num)
{
	num = (unsigned char)cur_mark_[0] + (((unsigned char)cur_mark_[1]) << 8);
	cur_mark_		+= 2;
	cur_buf_size_	+= 2;
}

void VBuffer::GetInteger(OUT int& num)
{
	num = ((unsigned char)cur_mark_[0] +
		(unsigned char)cur_mark_[1] << 8 +
		(unsigned char)cur_mark_[2] << 16 +
		(unsigned char)cur_mark_[3] << 24);
	cur_mark_		+= 4;
	cur_buf_size_	+= 4;
}

void VBuffer::GetString(OUT char* buf)
{
	short len;
	GetShort(len);
	if (len < 0 || len > MAX_PBUFSIZE)
	{
		return;
	}
	CopyMemory(buf, cur_mark_, len);
	buf[len] = '\0';
	cur_mark_		+= len;
	cur_buf_size_	+= len;
}

// 문자열 말고 다른 바이트 스트림을 읽을 때 쓰인다(?)
void VBuffer::GetStream(OUT char* buf, short len)
{
	if (len < 0 || len > MAX_PBUFSIZE)
	{
		return;
	}
	
	CopyMemory(buf, cur_mark_, len);
	cur_mark_		+= len;
	cur_buf_size_	+= len;
}

void VBuffer::SetChar(char ch)
{
	*cur_mark_++ = ch;
	cur_buf_size_ += 1;
}

void VBuffer::SetShort(short num)
{
	*cur_mark_++ = num;
	*cur_mark_++ = num >> 8;
	cur_buf_size_ += 2;
}

void VBuffer::SetInteger(int num)
{
	*cur_mark_++ = num;
	*cur_mark_++ = num >> 8;
	*cur_mark_++ = num >> 16;
	*cur_mark_++ = num >> 24;
	cur_buf_size_ += 4;
}

void VBuffer::SetString(char* buf)
{
	short len = strlen(buf);
	if (len < 0 || len > MAX_PBUFSIZE)
	{
		return;
	}
	SetShort(len);
	CopyMemory(cur_mark_, buf, len);
	cur_mark_		+= len;
	cur_buf_size_	+= len;
}

void VBuffer::SetStream(char* buf, short len)
{
	CopyMemory(cur_mark_, buf, len);
	cur_mark_		+= len;
	cur_buf_size_	+= len;
}

bool VBuffer::CopyBuffer(char* dest_buffer)
{
	CopyMemory(vbuf_, &cur_buf_size_, PACKET_LENGTH_BYTE);
	CopyMemory(dest_buffer, vbuf_, cur_buf_size_);
	return true;
}

void VBuffer::SetBuffer(char* vbuf)
{
	cur_mark_ = vbuf;
	cur_buf_size_ = 0;
}