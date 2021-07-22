#pragma once
#include <unistd.h>
#include <arpa/inet.h>
#include "Channel.hpp"
class TcpConnection
{
	typedef std::function<void()> CallBack;

public:
	TcpConnection(int fd):_fd(fd),_channel(new Channel(fd))
	{
		_channel->EnableRead();
	//	_channel->SetCloseCallback(std::bind(&CellServer::RemoveChannel, this, channel));
	//	_channel->SetReadCallBack(_ReadCallBack);
	}

	~TcpConnection()
	{
		delete _channel;
		close(_fd);
	}

	void setCloseCallBack(CallBack& cb)
	{
		_CloseCallBack = cb;
	}

	void setReadCallBack(CallBack& cb)
	{
		_ReadCallBack = cb;
	}

private:
	int _fd;
	Channel* _channel;
	CallBack _ReadCallBack;
	CallBack _CloseCallBack;
};

