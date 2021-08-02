#pragma once
#include <functional>
#include <iostream>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <signal.h>
#include "../base/buffer.hpp"
#include "../base//noncopyable.hpp"


using std::function;
using std::string;

class Connect:noncopyable
{
	typedef function<void()> CallBack;
	typedef function<void(Connect*, Buffer*)> MessageCallBack;
public:
	Connect(int fd) 
		:_fd(fd),_attention(EPOLLHUP | EPOLLERR),_buf(new Buffer())
	{}
	~Connect()
	{
		close(_fd);
 	}
	int fd(){return _fd;}
	uint32_t attention(){return _attention;}
	void Read();
	void enableRead(){_attention |= EPOLLIN;}
	void disableRead(){_attention ^= EPOLLIN;}
	void setReadCallBack(const CallBack& cb){_ReadCallBack = cb;}
	void setCloseCallback(const CallBack& cb){_CloseCallBack = cb;}
	void setMessageCallBack(const MessageCallBack& cb){_MessageCallBack = cb;}
	void Write(const char* message,int len)
	{
		send(_fd, message, len,MSG_DONTWAIT|MSG_NOSIGNAL);
	}
	void Write(string message){Write(message.c_str(), message.length());}
	void setEvents(int events){_event = events;}
	void handleEvent();

private:
	int _fd;
	Buffer* _buf;
	uint32_t _event;
	uint32_t _attention;
	CallBack _ReadCallBack;
	CallBack _CloseCallBack;
	MessageCallBack _MessageCallBack;

};

void Connect::handleEvent()
{
	if (_event & _attention & EPOLLIN)
	{
		Read();
	}
	else if (_event & _attention & (EPOLLHUP | EPOLLERR))
	{
		_CloseCallBack();
	}
	_event = 0;
}

void Connect::Read( )
{
	int n = _buf->readFD(fd());
	if (n > 0)
	{
		_MessageCallBack(this, _buf);
	}
	else _CloseCallBack();
}