#pragma once

namespace library
{
	class VBuffer : public Singleton<VBuffer>
	{
	public:
		VBuffer(int max_buf_size = MAX_VBUFSIZE);
		~VBuffer();

		void GetChar(OUT char& ch);
		void GetShort(OUT short& num);
		void GetInteger(OUT int& num);
		void GetString(OUT char* buf);
		void GetStream(OUT char* buf, short len);
		void SetChar(char ch);
		void SetShort(short num);
		void SetInteger(int num);
		void SetString(const char* buf);
		void SetStream(char* buf, short len);

		void SetBuffer(char* vbuf);
		inline char* GetBegin() { return vbuf_; }
		bool CopyBuffer(char* dest_buffer);
		void Init();

	private:
		char* vbuf_;
		char* cur_mark_;
		int		max_buf_size_;
		int		cur_buf_size_;
	};	
}