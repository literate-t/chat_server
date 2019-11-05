#include "stdafx.h"
#define SPIN_LOCK	100

LockGuard::LockGuard()
{
	(void)InitializeCriticalSectionAndSpinCount(&cs_, SPIN_LOCK);
}

LockGuard::~LockGuard()
{
	DeleteCriticalSection(&cs_);
}

void LockGuard::Enter()
{
	EnterCriticalSection(&cs_);
}

void LockGuard::Leave()
{
	LeaveCriticalSection(&cs_);
}

LockGuard::Owner::Owner(LockGuard& object):lock_(object)
{
	lock_.Enter();
}

LockGuard::Owner::~Owner()
{
	lock_.Leave();
}