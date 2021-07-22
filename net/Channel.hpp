#pragma once
#include <functional>
#include <iostream>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../base/buffer.hpp"

using std::function;

class Channel
{
	typedef function<void()> CallBack;
	typedef function<void(Channel*, Buffer*)> MessageCallBack;
public:
	Channel(int fd) :_fd(fd),_attention(EPOLLHUP | EPOLLERR),_buf(new Buffer())
	{}

	~Channel()
	{
		close(_fd);
		delete _buf;
 	}

	int fd()
	{
		return _fd;
	}
	
	uint32_t attention()
	{
		return _attention;
	}

	void EnableRead()
	{
		_attention |= EPOLLIN;
		
	}
	
	void DisableRead()
	{
		_attention ^= EPOLLIN;
	}

	void SetReadCallBack(const CallBack& cb)
	{
		_ReadCallBack = cb;
	}

	void SetCloseCallback(const CallBack& cb)
	{
		_CloseCallBack = cb;
	}

	void SetMessageCallBack(const MessageCallBack& cb)
	{
		_MessageCallBack = cb;
	}

	void Write(char* message,int len)
	{
		send(_fd, message, len, MSG_DONTWAIT);
	}

	void SetEvents(int events)
	{
		_event = events;
	}

	void HandleEvent()
	{
		if (_event & _attention & (EPOLLHUP || EPOLLERR))
		{
			if (_CloseCallBack) _CloseCallBack();
		}
		if (_event & _attention & EPOLLIN)
		{
			recvdata();
		}
		_event = 0;
	}

	void recvdata()
	{
		int n = _buf->readFD(_fd);
		if (n > 0)
		{
			_MessageCallBack(this, _buf);
			_buf->reset();
		}
		else _CloseCallBack();
	}

private:
	int _fd;
	Buffer* _buf;
	uint32_t _event;
	uint32_t _attention;
	CallBack _ReadCallBack;
	CallBack _CloseCallBack;
	MessageCallBack _MessageCallBack;
};

