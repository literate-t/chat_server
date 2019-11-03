#pragma once
#include "stdafx.h"

class Monitor
{
public:
	class Owner
	{
	public:
		Owner(Monitor& object);
		~Owner();

	private:
		Monitor& sync_object_;
		Owner(const Owner& rhs);
		Owner& operator=(const Owner& rhs);
	};

	Monitor();
	~Monitor();

#if(_WIN32_WINNT >= 0x0400)
	bool TryEnter();
#endif
	void Enter();
	void Leave();

private:
	CRITICAL_SECTION sync_object_;
	Monitor(const Monitor& rhs);
	Monitor& operator=(const Monitor& rhs);
};