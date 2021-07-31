#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <memory.h>
#include "Channel.hpp"
#include <sys/epoll.h>
#include "../base//noncopyable.hpp"

using std::vector;
using std::map;

const int EPOLL_SIZE = 10240;

class Epoller:noncopyable
{

public:
	Epoller(){_epfd = epoll_create(EPOLL_SIZE);}
	~Epoller();
	void epoll(int timeous, vector<Channel*>* activeChannels);
	void remove(Channel* channel);
	void add(Channel* channel);
private:
	void fillActiveChannels(int n, vector<Channel*>* activeChannels);
	int update(int op, Channel* channel);

	int _epfd;
	epoll_event _events[EPOLL_SIZE] = {};
	map<int, Channel*> _channels;
};

Epoller::~Epoller()
{
	for (auto it : _channels)
	{
		delete it.second;
	}
}

void Epoller::epoll(int timeous, vector<Channel*>* activeChannels)
{
	int n = epoll_wait(_epfd, _events, EPOLL_SIZE, timeous);
	fillActiveChannels(n, activeChannels);
}

void Epoller::remove(Channel* channel)
{
	update(EPOLL_CTL_DEL, channel);
	int fd = channel->fd();
	_channels.erase(fd);
}

void Epoller::add(Channel* channel)
{
	_channels[channel->fd()] = channel;
	update(EPOLL_CTL_ADD, channel);
}

void Epoller::fillActiveChannels(int n, vector<Channel*>* activeChannels)
{
	for (int i = 0; i < n; i++)
	{
		Channel* channel = static_cast<Channel*>(_events[i].data.ptr);
		channel->setEvents(_events[i].events);
		activeChannels->push_back(channel);
	}
}

int Epoller::update(int op, Channel* channel)
{
	epoll_event ev;
	ev.data.ptr = channel;
	ev.events = channel->attention();
	epoll_ctl(_epfd, op, channel->fd(), &ev);
}