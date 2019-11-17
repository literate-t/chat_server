#pragma once

#include "conmanip.h"
using namespace conmanip;

namespace ServerLibrary
{
	class Logger : public ILog, public Lock
	{
		using LockGurad = Lock::LockGuard;

	public:
		Logger()
		{
			console_out_context	console;
			ConsoleOut = console;
		}
		~Logger() = default;

		void Error(const char* text)
		{
			LockGuard lock(Cs);
			ConsoleOut.settextcolor(console_text_colors::red);
			printf("[ERROR] | %s\n", text);
		}

		void Warn(const char* text)
		{
			LockGuard lock(Cs);
			ConsoleOut.settextcolor(console_text_colors::yellow);
			printf("[WARN] | %s\n", text);
		}

		void Debug(const char* text)
		{
			LockGuard lock(Cs);
			ConsoleOut.settextcolor(console_text_colors::light_white);
			printf("[DEBUG] | %s\n", text);
		}

		void Trace(const char* text)
		{
			LockGuard lock(Cs);
			ConsoleOut.settextcolor(console_text_colors::light_white);
			printf("[TRACE] | %s\n", text);
		}

		void Info(const char* text)
		{
			LockGuard lock(Cs);
			ConsoleOut.settextcolor(console_text_colors::green);
			printf("[INFO] | %s\n", text);
		}

	private:
		console_out ConsoleOut;
		Lock Cs;		
	};
}
