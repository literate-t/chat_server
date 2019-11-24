#include "stdafx.h"

namespace ServerLibrary
{
	MessagePool::MessagePool(const int maxMsgPoolCount, const int extraMsgPoolCount)
	{
		MaxMsgQueueCount = maxMsgPoolCount;
		ExtraMsgQueueCount = extraMsgPoolCount;
		CreateMsgPool();
	}

	MessagePool::~MessagePool()
	{
		DestroyMsgPool();
	}

	void MessagePool::SetLog(ILog* log)
	{
		Log = log;
	}

	bool MessagePool::CheckCounts()
	{
		if (MaxMsgQueueCount == -1)
		{
			Log->Write(LogType::L_ERROR, "%s | MaxMsgQueueCount failure", __FUNCTION__);
			return false;
		}

		if (ExtraMsgQueueCount == -1)
		{
			Log->Write(LogType::L_ERROR, "%s | ExtraMsgQueueCount failure", __FUNCTION__);
			return false;
		}

		return true;
	}

	bool MessagePool::CreateMsgPool()
	{
		Message* msg = nullptr;
		for (int i = 0; i < MaxMsgQueueCount; ++i)
		{
			msg = new Message();
			msg->Clear();
			MessageQueue.push(msg);
		}
		for (int i = 0; i < ExtraMsgQueueCount; ++i)
		{
			msg = new Message();
			msg->Clear();
			MessageQueue.push(msg);
		}
		return true;
	}

	void MessagePool::DestroyMsgPool()
	{
		while (auto data = AllocateMsg())
		{
			delete data;
		}
	}

	Message* MessagePool::AllocateMsg()
	{
		Message* msg = nullptr;
		if (!MessageQueue.try_pop(msg))
		{
			return nullptr;
		}
		return msg;
	}

	bool MessagePool::DeallocateMsg(Message* msg)
	{
		if (msg == nullptr)
		{
			return false;
		}
		msg->Clear();
		MessageQueue.push(msg);
		return true;
	}
}