#pragma once

#include <atomic>
#include "noncopyable.hpp"
using std::atomic;

const int QUEUE_LENGTH = 204800;

template<typename T>
class lock_free_queue:noncopyable
{
public:
	lock_free_queue() :_head(0), _tail(0)
	{
		for (int i = 0; i < QUEUE_LENGTH; i++)
		{
			_valid[i] = false;
		}
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
		_head = (_head + 1) % QUEUE_LENGTH;
	}
private:
	int _head;
	atomic<int> _tail;
	bool _valid[QUEUE_LENGTH];
	T _queue[QUEUE_LENGTH];
};
