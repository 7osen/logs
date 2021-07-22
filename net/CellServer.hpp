#pragma once
#include <thread>
#include <iostream>
#include <mutex>
#include <thread>
#include "Epoller.hpp"
#include "TcpConnection.hpp"

using std::mutex;

class CellServer
{
	typedef std::function<void()> CallBack;
	typedef function<void(Channel*, Buffer*)> MessageCallBack;
public:
	CellServer()
		:_running(false), _epoller(), _connectnums(0)
	{
		
	}
	~CellServer()
	{

	}

	void start()
	{
		_running = true;
		_thread = new std::thread(std::bind(&CellServer::Run, this));
		_thread->detach();
	}

	void SetReadCallBack(CallBack& cb)
	{
		_ReadCallBack = cb;
	}

	void setMessageCallBack(const MessageCallBack& cb)
	{
		_MessageCallBack = cb;
	}

	void Run()
	{
		vector<Channel*> activeChannels;
		while (_running)
		{
			Update();
			activeChannels.clear();
			_epoller.epoll(1, &activeChannels);
			for (auto i : activeChannels)
			{
				i->HandleEvent();
			}
		}
	}

	void addNewClient(int fd)
	{
		std::lock_guard<mutex> lock(_mutex);
		_NewClient.push_back(fd);
		_connectnums++;
	}

	void RemoveChannel(Channel* channel)
	{
		std::lock_guard<mutex> lock(_mutex);
		_DeleteChannel.push_back(channel);
		_connectnums--;
	}

	int size()
	{
		return _connectnums;
	}

private:

	void Update()
	{
		std::lock_guard<mutex> lock(_mutex);
		for (auto i : _NewClient)
		{
			Channel* channel = new Channel(i);
			channel->EnableRead();
			channel->SetCloseCallback(std::bind(&CellServer::RemoveChannel, this, channel));
			channel->SetReadCallBack(_ReadCallBack);
			channel->SetMessageCallBack(_MessageCallBack);
			_epoller.Add(channel);
		}
		_NewClient.clear();
		
		for (auto channel : _DeleteChannel)
		{
			_epoller.Remove(channel);
			delete channel;
		}
		_DeleteChannel.clear();
	}

	bool _running;
	int _connectnums;
	mutex _mutex;
	CallBack _ReadCallBack;
	Epoller _epoller;
	std::thread* _thread;
	vector<int> _NewClient;
	vector<Channel*> _DeleteChannel;
	MessageCallBack _MessageCallBack;
};

