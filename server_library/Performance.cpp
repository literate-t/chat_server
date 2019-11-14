#include "stdafx.h"

namespace library
{
	Performance::~Performance()
	{
		if (IsPerformanceChecked)
		{
			SetCheckPerformance(false);
			PerformanceThread.join();
		}
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
			sprintf_s(logMsg, "Process packet count:%d, Millsecond:%d",PacketProcessCount, Milliseconds);
			Log.Write(LogType::L_INFO, "%s | %s", __FUNCTION__, logMsg);
			ResetPacketProcessCount();
		}
	}
}