#pragma once

template <typename T>
class Singleton
{
public:
	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);

	static T& GetInstance()
	{
		return instance_;
	}

protected:
	Singleton() {};
	~Singleton() {};

private:
		static T instance_;
};