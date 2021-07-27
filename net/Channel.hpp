#pragma once
#include <functional>
#include <iostream>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include "../base/buffer.hpp"
#include "../base//noncopyable.hpp"

using std::function;
using std::string;

class Channel:noncopyable
{
	typedef function<void()> CallBack;
	typedef function<void(Channel*, Buffer*)> MessageCallBack;
public:
	Channel(int fd) 
		:_fd(fd),_attention(EPOLLHUP | EPOLLERR),_buf(new Buffer())
	{}
	~Channel()
	{
		close(_fd);
 	}
	int fd(){return _fd;}
	uint32_t attention(){return _attention;}
	void Read();
	void EnableRead(){_attention |= EPOLLIN;}
	void DisableRead(){_attention ^= EPOLLIN;}
	void SetReadCallBack(const CallBack& cb){_ReadCallBack = cb;}
	void SetCloseCallback(const CallBack& cb){_CloseCallBack = cb;}
	void SetMessageCallBack(const MessageCallBack& cb){_MessageCallBack = cb;}
	void Write(const char* message,int len){send(_fd, message, len, MSG_DONTWAIT);}
	void Write(string message){Write(message.c_str(), message.length());}
	void SetEvents(int events){_event = events;}
	void HandleEvent();
	void recvdata();

private:
	int _fd;
	Buffer* _buf;
	uint32_t _event;
	uint32_t _attention;
	CallBack _ReadCallBack;
	CallBack _CloseCallBack;
	MessageCallBack _MessageCallBack;

};

void Channel::HandleEvent()
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

void Channel::Read( )
{
	int n = _buf->readFD(fd());
	if (n > 0)
	{
		_MessageCallBack(this, _buf);
	}
	else _CloseCallBack();
}