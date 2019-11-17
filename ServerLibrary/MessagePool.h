#pragma once

namespace ServerLibrary
{
	class MessagePool
	{
	public:
		explicit MessagePool(const int maxMsgQueueCount, const int extraMsgQueueCount);
		~MessagePool();

	public:
		bool CheckCounts();
		Message* AllocateMsg();
		bool DeallocateMsg(Message* msg);
		void SetLog(ILog* log);

	private:
		bool CreateMsgPool();
		void DestroyMsgPool();		

	private:
		concurrency::concurrent_queue<Message*> MessageQueue;
		int MaxMsgQueueCount = -1;
		int ExtraMsgQueueCount = -1;
		ILog* Log;
	};
}