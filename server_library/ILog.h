#pragma once
#include <stdio.h>
#include <stdarg.h>

namespace server_library
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

	class ILog
	{
	public:
		ILog()			= default;
		virtual ~ILog() = default;

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

	protected:
		virtual	void Error(const char* text) = 0;
		virtual	void Warn(const char* text) = 0;
		virtual void Debug(const char* text) = 0;
		virtual void Trace(const char* text) = 0;
		virtual void Info(const char* text) = 0;
	};
}