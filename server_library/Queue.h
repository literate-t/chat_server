#pragma once
#include "stdafx.h"

template <typename T>
class Queue : public LockGuard
{
	Queue(int max_size = MAX_QUEUESIZE);
	~Queue();

	bool Push(T item);
	void Pop();
	bool IsEmpty();
	bool Front();
	int GetQueueSize();
};