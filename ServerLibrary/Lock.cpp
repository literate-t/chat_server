#include "stdafx.h"
#define SPIN_LOCK	100

namespace ServerLibrary
{
	Lock::Lock()
	{
		(void)InitializeCriticalSectionAndSpinCount(&cs_, SPIN_LOCK);
	}

	Lock::~Lock()
	{
		DeleteCriticalSection(&cs_);
	}

	void Lock::Enter()
	{
		EnterCriticalSection(&cs_);
	}

	void Lock::Leave()
	{
		LeaveCriticalSection(&cs_);
	}

	Lock::LockGuard::LockGuard(Lock& object):locker_(object)
	{
		locker_.Enter();
	}

	Lock::LockGuard::~LockGuard()
	{
		locker_.Leave();
	}
}