#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <memory.h>
#include "Connect.hpp"
#include <sys/epoll.h>
#include "../base/noncopyable.hpp"

using std::vector;
using std::map;

const int EPOLL_SIZE = 10240;

class Epoller:noncopyable
{

public:
	Epoller(){_epfd = epoll_create(EPOLL_SIZE);}
	~Epoller();
	void epoll(int timeous, vector<Connect*>* activeChannels);
	void remove(Connect* channel);
	void add(Connect* channel);
private:
	void fillActiveChannels(int n, vector<Connect*>* activeChannels);
	int update(int op, Connect* channel);

	int _epfd;
	epoll_event _events[EPOLL_SIZE] = {};
	map<int, Connect*> _channels;
};

Epoller::~Epoller()
{
	for (auto it : _channels)
	{
		delete it.second;
	}
}

void Epoller::epoll(int timeous, vector<Connect*>* activeChannels)
{
	int n = epoll_wait(_epfd, _events, EPOLL_SIZE, timeous);
	fillActiveChannels(n, activeChannels);
}

void Epoller::remove(Connect* channel)
{
	update(EPOLL_CTL_DEL, channel);
	int fd = channel->fd();
	_channels.erase(fd);
}

void Epoller::add(Connect* channel)
{
	_channels[channel->fd()] = channel;
	update(EPOLL_CTL_ADD, channel);
}

void Epoller::fillActiveChannels(int n, vector<Connect*>* activeChannels)
{
	for (int i = 0; i < n; i++)
	{
		Connect* channel = static_cast<Connect*>(_events[i].data.ptr);
		channel->setEvents(_events[i].events);
		activeChannels->push_back(channel);
	}
}

int Epoller::update(int op, Connect* channel)
{
	epoll_event ev;
	ev.data.ptr = channel;
	ev.events = channel->attention();
	epoll_ctl(_epfd, op, channel->fd(), &ev);
}