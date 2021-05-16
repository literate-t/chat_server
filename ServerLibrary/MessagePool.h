#pragma once

namespace ServerLibrary
{
	class ILog;
	class MessagePool
	{
	public:
		explicit MessagePool(const int max_msg_queue_count, const int extra_msg_queue_count);
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
		concurrency::concurrent_queue<Message*> message_queue_;
		int max_msg_queue_count_ = -1;
		int extra_max_msg_queue_count_ = -1;
		ILog* log_;
	};
}