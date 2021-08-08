#pragma once
#include <thread>
#include "database.hpp"
#include "noncopyable.hpp"
using std::thread;

class writer:noncopyable
{
public:
	writer(database* st)
		:_storager(st),_queue(new mq<message>(QUEUE_LENGTH))
	{

	}

	void start()
	{
		_writerThread = new std::thread(std::bind(&writer::write, this));
		_writerThread->detach();
	}

	void set(string timestamp, string topic, string context)
	{
		_queue->push(message(timestamp, topic, context));
	}

	~writer()
	{
		delete _queue;
	}

private:
	void write()
	{
		int count = 0;
		TimeCount t;
		t.Update();
		while (1)
		{
			_storager->set(_queue->front());
			count++;
			if (t.getSecond() > 1.0)
			{
				printf("%d\n", count);
				count = 0;
				t.Update();
			}
			_queue->pop();
		}
	}

	database* _storager;
	thread* _writerThread;
	mq<message>* _queue;
};
