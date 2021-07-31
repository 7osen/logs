#pragma once
#include "../base/storager.hpp"
#include "../base/lock_free_queue.hpp"
#include "HttpServer.hpp"
#include "../base//timecount.hpp"
#include <atomic>

const char* TIMESTRING = "time";
const char* TOPICSTRING = "topic";
const char* CONTEXTSTRING = "context";



class LogServer:noncopyable
{
public:
	LogServer(int port,int threadnums = 1)
		:_server(port, threadnums)
	{
		_queue = new lock_free_queue<message>();
		_server.setPostCallBack(std::bind(&LogServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
		_server.setGetCallBack(std::bind(&LogServer::find, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	}

	~LogServer()
	{

	}

	void start()
	{
		_thread = new std::thread(std::bind(&LogServer::run, this));
		_thread->detach();
		_server.start();
	}

	void close()
	{
		_server.close();
	}

private:
	void find(string& st, const string& username, const string& begin, const string& end,int num = 10000)
	{
		std::stringstream* ss = new std::stringstream();
		_storager.get(ss, username, begin, end,num);
		st = ss->str();
		delete ss;
	}

	void onMessage(char* begin, char* end)
	{
		string timestamp, topic, context;
		char* mid = std::find(begin, end, '=');
		for (; mid != end; mid = std::find(begin, end, '='))
		{
			char* next = std::find(mid + 1, end, '&');
			char* val = mid + 1;
			int vallen = next - val;
			if (strcmp(begin, mid - begin, TIMESTRING))
			{
				timestamp.assign(val, next);
			}
			else if (strcmp(begin, mid - begin, TOPICSTRING))
			{
				topic.assign(val, next);
			}
			else if (strcmp(begin, mid - begin, CONTEXTSTRING))
			{
				context.assign(val, next);
			}
			begin = next + 1;
		}
		set(timestamp, topic, context);
	}
	bool strcmp(char* ch, int len, const char* src)
	{
		for (int i = 0; i < len; i++)
		{
			if (ch[i] != src[i]) return false;
		}
		return true;
	}

	void set(string timestamp, string topic, string context)
	{
		message m = message(timestamp,  topic,  context);
		_queue->push(std::move(m));
	}

	void run()
	{
		int count = 0;
		TimeCount t;
		t.Update();
		while (1)
		{
			if (t.getSecond() > 1.0)
			{
				printf("%d\n", count);
				count = 0;
				t.Update();
			}
			_storager.set(_queue->front());
			count++;
			_queue->pop();
		}
	}

	HttpServer _server;
	storager _storager;
	std::thread* _thread;
	lock_free_queue<message>* _queue;
};

