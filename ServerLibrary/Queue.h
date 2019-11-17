#pragma once

namespace library
{
	template <typename T>
	class Queue : public Lock
	{
		using LockGuard = Lock::LockGuard;
	public:
		explicit Queue(int max_size);
		~Queue();

		bool Push(T item);
		void Pop();
		bool IsEmpty();
		T Front();
		int GetQueueSize();
		int GetQueueMaxSize() { return queue_max_size_; }
		void SetQueueMaxSize(int max_size) { queue_max_size_ = max_size; }
		void Clear();

	private:
		T* array_queue_;
		int			queue_max_size_;
		int			push_pos_;
		int			front_pos_;
		int			size_;
		Lock		cs_;
	};

	template <typename T>
	Queue<T>::Queue(int max_size)
	{
		array_queue_ = new T[max_size];
		queue_max_size_ = max_size;
		Clear();
	}

	template <typename T>
	Queue<T>::~Queue()
	{
		delete[] array_queue_;
	}

	template <typename T>
	bool Queue<T>::Push(T item)
	{
		LockGuard lock(cs_);
		if (push_pos_ >= queue_max_size_)
		{
			return false;
		}

		else if (push_pos_ == queue_max_size_)
		{
			push_pos_ = 0;
		}
		array_queue_[push_pos_++] = item;
		++size_;
		return true;
	}

	template <typename T>
	void Queue<T>::Pop()
	{
		LockGuard lock(cs_);
		++front_pos_;
		assert(front_pos_ <= queue_max_size_);
		--size_;
		if (front_pos_ == queue_max_size_)
		{
			front_pos_ = 0;
		}
	}

	template <typename T>
	T Queue<T>::Front()
	{
		LockGuard lock(cs_);
		if (push_pos_ <= 0)
		{
			return 0;
		}
		if (front_pos_ >= queue_max_size_)
		{
			front_pos_ = 0;
		}
		T& item = array_queue_[front_pos_];
		return item;
	}

	template <typename T>
	bool Queue<T>::IsEmpty()
	{
		LockGuard lock(cs_);
		bool result = size_ > 0 ? false : true;
		return result;
	}

	template <typename T>
	int Queue<T>::GetQueueSize()
	{
		LockGuard lock(cs_);
		// push_pos 값이 데이터 개수와 같다
		return push_pos_;
	}

	template <typename T>
	void Queue<T>::Clear()
	{
		LockGuard lock(cs_);
		push_pos_ = 0;
		front_pos_ = 0;
		size_ = 0;
	}
}