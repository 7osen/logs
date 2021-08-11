#pragma once
#include <thread>
#include <iostream>
#include <mutex>
#include <thread>
#include "Epoller.hpp"


const int MaxConnectNum = 4096;

using std::mutex;

class WorkerThread
{
	typedef std::function<void()> CallBack;
	typedef function<void(Connect*, Buffer*)> MessageCallBack;
public:
	WorkerThread()
		:_quit(false), _epoller(), _connectnums(0){}
	~WorkerThread(){ delete _thread; }
	int size(){ return _connectnums;}
	void close(){ _quit = true;}
	bool isClosed(){ return !_running;}
	void start();
	void setReadCallBack(CallBack& cb){ _ReadCallBack = cb;}
	void setMessageCallBack(const MessageCallBack& cb){ _MessageCallBack = cb;}
	void addNewClient(int fd);
	void removeChannel(Connect* channel);

private:
	void update();
	void run();

	bool _running;
	bool _quit;
	int _connectnums;
	mutex _mutex;
	CallBack _ReadCallBack;
	Epoller _epoller;
	std::thread* _thread;
	vector<int> _NewClient;
	vector<Connect*> _DeleteChannel;
	MessageCallBack _MessageCallBack;
};

void WorkerThread::start()
{
	_thread = new std::thread(std::bind(&WorkerThread::run, this));
	_thread->detach();
}

void WorkerThread::addNewClient(int fd)
{
	std::lock_guard<mutex> lock(_mutex);
	_NewClient.push_back(fd);
	_connectnums++;
}

void WorkerThread::removeChannel(Connect* channel)
{
	std::cout << "exit" << std::endl;
	std::lock_guard<mutex> lock(_mutex);
	_DeleteChannel.push_back(channel);
	_connectnums--;
}

void WorkerThread::update()
{
	std::lock_guard<mutex> lock(_mutex);
	for (auto i : _NewClient)
	{
		Connect* channel = new Connect(i);
		channel->enableRead();
		channel->setCloseCallback(std::bind(&WorkerThread::removeChannel, this, channel));
		channel->setMessageCallBack(_MessageCallBack);
		_epoller.add(channel);
	}
	_NewClient.clear();

	for (auto channel : _DeleteChannel)
	if (channel)
	{
		::close(channel->fd());
		_epoller.remove(channel);
		delete channel;
	}
	_DeleteChannel.clear();
}

void WorkerThread::run()
{
	_running = true;
	vector<Connect*> activeChannels;
	activeChannels.reserve(MaxConnectNum);
	while (!_quit)
	{
		update();
		activeChannels.clear();
		_epoller.epoll(1, &activeChannels);
		for (auto i : activeChannels)
		{
			i->handleEvent();
		}
	}
	_running = false;
}

