#include "stdafx.h"

namespace server_library
{
	MessagePool::MessagePool(const int max_msg_pool_count, const int extra_msg_pool_count)
	{
		max_msg_queue_count_ = max_msg_pool_count;
		extra_max_msg_queue_count_ = extra_msg_pool_count;
		CreateMsgPool();
	}

	MessagePool::~MessagePool()
	{
		DestroyMsgPool();
	}

	void MessagePool::SetLog(ILog* log)
	{
		log_ = log;
	}

	bool MessagePool::CheckCounts()
	{
		if (max_msg_queue_count_ == -1)
		{
			log_->Write(LogType::L_ERROR, "%s | max_msg_queue_count_ failure", __FUNCTION__);
			return false;
		}

		if (extra_max_msg_queue_count_ == -1)
		{
			log_->Write(LogType::L_ERROR, "%s | extra_max_msg_queue_count_ failure", __FUNCTION__);
			return false;
		}

		return true;
	}

	bool MessagePool::CreateMsgPool()
	{
		Message* msg = nullptr;
		for (int i = 0; i < max_msg_queue_count_; ++i)
		{
			msg = new Message();
			msg->Clear();
			message_queue_.push(msg);
		}
		for (int i = 0; i < extra_max_msg_queue_count_; ++i)
		{
			msg = new Message();
			msg->Clear();
			message_queue_.push(msg);
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
		if (!message_queue_.try_pop(msg))
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
		message_queue_.push(msg);
		return true;
	}
}