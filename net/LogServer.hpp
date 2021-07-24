#pragma once
#include "../base/kv.hpp"
#include "../base/lock_free_queue.hpp"
#include "HttpServer.hpp"
#include <atomic>

const char* TIMESTRING = "time";
const char* USERNAMESTRING = "username";
const char* TOPICSTRING = "topic";
const char* CONTEXTSTRING = "context";
enum keyType
{
	error = 0,
	timestamp = 1,
	username = 2,
	topic = 3,
	context = 4
};


class LogServer
{
public:
	LogServer(int port,int threadnums = 1)
		:_server(port, threadnums)
	{
		_server.setOnMessageCallBack(std::bind(&LogServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
	}

	~LogServer()
	{
	}

	void start()
	{
		_thread = new std::thread(std::bind(&LogServer::Run, this));
		_thread->detach();
		_server.start();
	}

	void close()
	{
		_server.close();
	}

	void onMessage(char* begin, char* end)
	{
		char *timestamp,*userid,*topic, *context;
		int timestamplen, useridlen, topiclen, contextlen;
		char* mid = std::find(begin, end, '-');
		for (; mid != end; mid = std::find(begin, end, '-'))
		{
			char* next = std::find(next + 1, end, '&');
			char* val = mid + 1;
			int vallen = next - val;
			if (strcmp(begin, mid - begin, TIMESTRING))
			{
				timestamp = val;
				timestamplen = vallen;
			}
			else if (strcmp(begin, mid - begin, USERNAMESTRING))
			{
				userid = val;
				useridlen = vallen;
			}
			else if (strcmp(begin, mid - begin, TOPICSTRING))
			{
				topic = val;
				topiclen = vallen;
			}
			else if (strcmp(begin, mid - begin, CONTEXTSTRING))
			{
				context = val;
				contextlen = vallen;
			}
		}
		Set(timestamp, timestamplen, userid, useridlen, topic, topiclen, context, contextlen);
	}

private:
	bool strcmp(char* ch, int len, const char* src)
	{
		for (int i = 0; i < len; i++)
		{
			if (ch[i] != src[i]) return false;
		}
		return true;
	}

	void Set(char* timestamp, int timestamplen, char* userid, int useridlen, char* topic, int topiclen, char* context, int contextlen)
	{
		message* m = new message(timestamp, timestamplen, userid, useridlen, topic, topiclen, context, contextlen);
		_queue.push(m);
	}

	void Run()
	{
		_kv.Set(*_queue.front());
		_queue.pop();
	}

	HttpServer _server;
	Kv _kv;
	std::thread* _thread;
	lock_free_queue<message*> _queue;
};

