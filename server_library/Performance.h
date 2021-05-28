#pragma once

namespace ServerLibrary
{
	class ILog;
	class Performance
	{
	public:
		Performance() = default;
		~Performance();

	public:
		void Start(int millisecond = 0);
		void CheckPerformanceThread();
		int IncrementPacketProcessCount() { return IsPerformanceChecked ? ++PacketProcessCount : 0; }
		void SetLog(ILog* log);

	private:
		void SetCheckPerformance(bool performanceChecked) { IsPerformanceChecked = performanceChecked; }
		void ResetPacketProcessCount() { PacketProcessCount = 0; }

	private:
		thread			PerformanceThread;
		atomic<int>		PacketProcessCount = 0;
		atomic<bool>	IsPerformanceChecked = false;
		int				Milliseconds = 0;

		ILog* Log = nullptr;
	};
}