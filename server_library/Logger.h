#pragma once

#include "conmanip.h"
using namespace conmanip;

namespace library
{
	const int kMaxLogStringLength = 256;
	enum class LogType : short
	{
		L_TRACE = 1,
		L_DEBUG = 2,
		L_WARN = 3,
		L_ERROR = 4,
		L_INFO = 5
	};

	class Logger : public Lock
	{
	private:
		console_out console_out_;
		Lock cs_;
		using LockGurad = Lock::LockGuard;

	public:
		Logger()
		{
			console_out_context	console;
			console_out_ = console;
		}

		~Logger() {}

		void Write(const LogType type, const char* format, ...)
		{
			char text[kMaxLogStringLength];
			va_list args;			// 가변 인자 목록 포인터
			va_start(args, format); // 가변 인자 목록 포인터 설정
			vsprintf_s(text, kMaxLogStringLength, format, args);
			va_end(args);

			switch (type)
			{
			case LogType::L_INFO:
			{
				Info(text);
				break;
			}

			case LogType::L_ERROR:
			{
				Error(text);
				break;
			}

			case LogType::L_WARN:
			{
				Warn(text);
				break;
			}

			case LogType::L_DEBUG:
			{
				Debug(text);
				break;
			}

			case LogType::L_TRACE:
			{
				Info(text);
				break;
			}
			}
		}

		void Error(const char* text)
		{
			LockGuard lock(cs_);
			console_out_.settextcolor(console_text_colors::red);
			printf("[ERROR] | %s\n", text);
		}

		void Warn(const char* text)
		{
			LockGuard lock(cs_);
			console_out_.settextcolor(console_text_colors::yellow);
			printf("[WARN] | %s\n", text);
		}

		void Debug(const char* text)
		{
			LockGuard lock(cs_);
			console_out_.settextcolor(console_text_colors::light_white);
			printf("[DEBUG] | %s\n", text);
		}

		void Trace(const char* text)
		{
			LockGuard lock(cs_);
			console_out_.settextcolor(console_text_colors::light_white);
			printf("[TRACE] | %s\n", text);
		}

		void Info(const char* text)
		{
			LockGuard lock(cs_);
			console_out_.settextcolor(console_text_colors::green);
			printf("[INFO] | %s\n", text);
		}
	};
}
