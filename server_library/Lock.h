#pragma once

namespace library
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
			Lock& lock_;
		};

		Lock();
		~Lock();

		void Enter();
		void Leave();

	private:
		CRITICAL_SECTION cs_;
	};
}
