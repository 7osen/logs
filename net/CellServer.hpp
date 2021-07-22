#pragma once
#include <thread>
#include <iostream>

class CellServer;

thread_local CellServer* _this_thread_eventloop_ = 0;

class CellServer
{
public:
	CellServer()
		:_looping(false)
	{
		if (_this_eventloop_)
		{
			std::cout << "this thread has EventLoop already!" << std::endl;
		}
		else
		{
			_this_thread_eventloop_ = this;
		}
	}
	~CellServer()
	{
		_this_thread_eventloop_ = 0;
	}

private:
	bool _looping;

};

