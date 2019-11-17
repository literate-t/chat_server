#include "stdafx.h"

namespace ServerLibrary
{
	Performance::~Performance()
	{
		if (IsPerformanceChecked)
		{
			SetCheckPerformance(false);
			PerformanceThread.join();
		}
	}

	void Performance::SetLog(ILog* log)
	{
		Log = log;
	}

	void Performance::Start(int milliseconds)
	{
		Milliseconds = milliseconds;
		if (Milliseconds > 0)
		{
			SetCheckPerformance(true);
			PerformanceThread = thread(&Performance::CheckPerformanceThread, this);
		}
	}

	void Performance::CheckPerformanceThread()
	{
		while (true)
		{
			this_thread::sleep_for(chrono::milliseconds(Milliseconds));
			if (IsPerformanceChecked == false)
			{
				return;
			}

			char logMsg[64] = { 0 };
			Log->Write(LogType::L_INFO, "%s | Process packet count:%d, Millsecond:%d", __FUNCTION__, PacketProcessCount.load(), Milliseconds);
			ResetPacketProcessCount();
		}
	}
}