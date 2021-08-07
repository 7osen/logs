#pragma once

#include <atomic>
#include "noncopyable.hpp"
#include "semaphore.hpp"
using std::atomic;

const int QUEUE_LENGTH = 40960;

template<typename T>
class mq:noncopyable
{
public:
	mq(int size = QUEUE_LENGTH) :_size(size),_head(0), _tail(0)
	{
		for (int i = 0; i < size; i++)
		{
			_valid[i] = false;
		}
	}

	~mq(){}

	void push(const T& val)
	{
		int old = _tail;
		int next = (old + 1) % _size;
		while (next == _head) sleep(1);
		while (!_tail.compare_exchange_weak(old, next))
		{
			next = (old + 1) % _size;
			while (next == _head) sleep(1);
		}
		_queue[old] = val;
		_valid[old] = true;
	}

	T& front()
	{
		while (_head == _tail);
		while (!_valid[_head]);
		return _queue[_head];
	}

	void pop()
	{
		_valid[_head] = false;
		_head = (_head + 1) % _size;
	}
private:
	int _head;
	int _size;
	atomic<int> _tail;
	bool _valid[QUEUE_LENGTH];
	T _queue[QUEUE_LENGTH];
};
