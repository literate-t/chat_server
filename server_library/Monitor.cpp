#include "stdafx.h"
#include "Monitor.h"

Monitor::Monitor()
{
	InitializeCriticalSection(&sync_object_);
}

Monitor::~Monitor()
{
	DeleteCriticalSection(&sync_object_);
}

#if(_WIN32_WINNT >= 0x0400)
bool Monitor::TryEnter()
{
	return TryEnterCriticalSection(&sync_object_);
}
#endif

void Monitor::Enter()
{
	EnterCriticalSection(&sync_object_);
}

void Monitor::Leave()
{
	LeaveCriticalSection(&sync_object_);
}

Monitor::Owner::Owner(Monitor& object):sync_object_(object)
{
	sync_object_.Enter();
}

Monitor::Owner::~Owner()
{
	sync_object_.Leave();
}