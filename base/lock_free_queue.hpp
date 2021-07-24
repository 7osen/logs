#pragma once


#include <atomic>
using std::atomic;

const int QUEUE_LENGTH = 40960;

template<typename T>
class lock_free_queue
{
public:
	lock_free_queue():_head(0),_tail(0)
	{

	}

	~lock_free_queue()
	{

	}

	void push(const T& val)
	{
		int next = (_tail + 1) % QUEUE_LENGTH;
		while (next == _head);
		int old = _tail;
		while (!_tail.compare_exchange_weak(old, next))
		{
			next = (old + 1) % QUEUE_LENGTH;
			while (next == _head);
		}
		_queue[old] = val;
	}

	T front()
	{
		while (_head == _tail);
		return _queue[_head];
	}

	void pop()
	{
		_head = (_head + 1) % QUEUE_LENGTH;
	}
private:
	T _queue[QUEUE_LENGTH];
	int _head;
	atomic<int> _tail;
};
