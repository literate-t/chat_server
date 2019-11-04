#pragma once

class Lock
{
public:
	class Owner
	{
	public:
		Owner(Lock& object);
		~Owner();

	private:
		Lock& sync_object_;
		Owner(const Owner& rhs);
		Owner& operator=(const Owner& rhs);
	};

	Lock();
	~Lock();

#if(_WIN32_WINNT >= 0x0400)
	bool TryEnter();
#endif
	void Enter();
	void Leave();

private:
	CRITICAL_SECTION sync_object_;
	Lock(const Lock& rhs);
	Lock& operator=(const Lock& rhs);
};