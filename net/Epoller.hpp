#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <memory.h>
#include "Channel.hpp"

#if __linux__
#include <sys/epoll.h>
#endif

using std::vector;

const int EPOLL_SIZE = 10240;

class Epoller
{
public:
	Epoller()
	{
		_epfd = epoll_create(EPOLL_SIZE);
	}

	~Epoller(){}

	void epoll()
	{
		int n = epoll_wait(_epfd, _events, EPOLL_SIZE, 1);
		for (int i = 0; i < n; i++)
		{
			auto channel = _mp[_events[i].data.fd];
			channel->SetEvents(_events[i].events);
			channel->HandleEvent();
		}
	}

	void Remove(int fd)
	{

	}
	
	void Update(int op, )
	{

	}

private:
	int _epfd;
	epoll_event _events[EPOLL_SIZE] = {};
	map<int, shared_ptr<Channel> > _mp;
};

