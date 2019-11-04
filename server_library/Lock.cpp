#include "stdafx.h"

Lock::Lock()
{
	InitializeCriticalSection(&sync_object_);
}

Lock::~Lock()
{
	DeleteCriticalSection(&sync_object_);
}

#if(_WIN32_WINNT >= 0x0400)
bool Lock::TryEnter()
{
	return TryEnterCriticalSection(&sync_object_);
}
#endif

void Lock::Enter()
{
	EnterCriticalSection(&sync_object_);
}

void Lock::Leave()
{
	LeaveCriticalSection(&sync_object_);
}

Lock::Owner::Owner(Lock& object):sync_object_(object)
{
	sync_object_.Enter();
}

Lock::Owner::~Owner()
{
	sync_object_.Leave();
}