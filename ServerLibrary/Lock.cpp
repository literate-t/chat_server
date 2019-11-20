#include "stdafx.h"
//#include "Lock.h"
#define SPIN_LOCK	100

namespace ServerLibrary
{
	Lock::Lock()
	{
		(void)InitializeCriticalSectionAndSpinCount(&Cs, SPIN_LOCK);
	}

	Lock::~Lock()
	{
		DeleteCriticalSection(&Cs);
	}

	void Lock::Enter()
	{
		EnterCriticalSection(&Cs);
	}

	void Lock::Leave()
	{
		LeaveCriticalSection(&Cs);
	}

	Lock::LockGuard::LockGuard(Lock& object) :Locker(object)
	{
		Locker.Enter();
	}

	Lock::LockGuard::~LockGuard()
	{
		Locker.Leave();
	}
}