#pragma once

class LockGuard
{
public:
	class Owner
	{
	public:
		Owner(LockGuard& object);
		~Owner();

	private:
		LockGuard& lock_;
	};

	LockGuard();
	~LockGuard();

	void Enter();
	void Leave();

private:
	CRITICAL_SECTION cs_;
};