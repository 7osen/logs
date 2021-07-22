#pragma once
#include <functional>
#include <iostream>
#if __linux__
 #include <sys/epoll.h>
 #include <sys/poll.h>
#endif // __linux__

class EventLoop;

using std::function;

class Channel
{
	typedef function<void()> CallBack;
public:
	Channel(EventLoop* loop,int fd) :_loop(loop),_fd(fd){}
	~Channel(){}
	void EnableRead()
	{
		_attention |= EPOLLIN;
	}
	
	void DisableRead()
	{
		_attention ^= EPOLLIN;
	}

	void SetReadHandle(const CallBack& cb)
	{
		_ReadHandle = cb;
	}

	void SetEvents(int events)
	{
		_event = events;
	}

	void HandleEvent()
	{
		if (_event & _attention & EPOLLIN)
			if (_ReadHandle) _ReadHandle();
	}

private:
	int _fd;
	int _event;
	int _attention;
	EventLoop* _loop;
	CallBack _ReadHandle;
};

