#pragma once

template <typename T>
class Singleton
{
public:
	static T* GetInstance()
	{
		return &instance_;
	}

protected:
	Singleton() {};
	~Singleton() {};

private:
	static T instance_;
};

template <typename T> T Singleton<T>::instance_;