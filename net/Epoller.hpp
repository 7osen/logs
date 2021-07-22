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
using std::map;

const int EPOLL_SIZE = 10240;

class Epoller
{

public:
	Epoller()
	{
		_epfd = epoll_create(EPOLL_SIZE);
	}

	~Epoller(){}

	void epoll(int timeous,vector<Channel*>* activeChannels)
	{
		int n = epoll_wait(_epfd, _events, EPOLL_SIZE, timeous);
		fillActiveChannels(n, activeChannels);
	}

	void Remove(Channel* channel)
	{
		Update(EPOLL_CTL_DEL, channel);
		_channels.erase(channel->fd());
	}
	

	void Add(Channel* channel)
	{
		_channels[channel->fd()] = channel;
		Update(EPOLL_CTL_ADD, channel);
	}


private:
	void fillActiveChannels(int n, vector<Channel*>* activeChannels)
	{
		for (int i = 0; i < n; i++)
		{
			Channel* channel = static_cast<Channel*>(_events[i].data.ptr);
			channel->SetEvents(_events[i].events);
			activeChannels->push_back(channel);
		}
	}

	int Update(int op, Channel* channel)
	{
		epoll_event ev;
		ev.data.ptr = channel;
		ev.events = channel->attention();
		epoll_ctl(_epfd, op, channel->fd(), &ev);
	}

	int _epfd;
	epoll_event _events[EPOLL_SIZE] = {};
	map<int, Channel*> _channels;
};

