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
		:_quit(false), _epoller(), _connectnums(0){}
	~CellServer(){ delete _thread; }
	int size(){ return _connectnums;}
	void close(){ _quit = true;}
	bool isClosed(){ return _running;}
	void start();
	void SetReadCallBack(CallBack& cb){ _ReadCallBack = cb;}
	void setMessageCallBack(const MessageCallBack& cb){ _MessageCallBack = cb;}
	void addNewClient(int fd);
	void RemoveChannel(Channel* channel);


private:
	void Update();
	void Run();

	bool _running;
	bool _quit;
	int _connectnums;
	mutex _mutex;
	CallBack _ReadCallBack;
	Epoller _epoller;
	std::thread* _thread;
	vector<int> _NewClient;
	vector<Channel*> _DeleteChannel;
	MessageCallBack _MessageCallBack;
};

void CellServer::start()
{
	_thread = new std::thread(std::bind(&CellServer::Run, this));
	_thread->detach();
}

void CellServer::addNewClient(int fd)
{
	std::lock_guard<mutex> lock(_mutex);
	_NewClient.push_back(fd);
	_connectnums++;
}

void CellServer::RemoveChannel(Channel* channel)
{
	std::cout << "exit" << std::endl;
	std::lock_guard<mutex> lock(_mutex);
	_DeleteChannel.push_back(channel);
	_connectnums--;
}

void CellServer::Update()
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

void CellServer::Run()
{
	_running = true;
	vector<Channel*> activeChannels;
	while (!_quit)
	{
		Update();
		activeChannels.clear();
		_epoller.epoll(1, &activeChannels);
		for (auto i : activeChannels)
		{
			i->HandleEvent();
		}
	}
	_running = false;
}

