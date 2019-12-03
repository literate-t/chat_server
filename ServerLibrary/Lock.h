#pragma once

namespace ServerLibrary
{
	class Lock
	{
	public:
		class LockGuard
		{
		public:
			LockGuard(Lock& object);
			~LockGuard();

		private:
			Lock& Locker;
		};

		Lock();
		~Lock();

		void Enter();
		void Leave();

	private:
		CRITICAL_SECTION Cs;
	};
}