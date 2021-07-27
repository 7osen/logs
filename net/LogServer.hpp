#pragma once
#include "../base/storager.hpp"
#include "../base/lock_free_queue.hpp"
#include "HttpServer.hpp"
#include "../base//timecount.hpp"
#include <atomic>

const char* TIMESTRING = "time";
const char* USERNAMESTRING = "username";
const char* TOPICSTRING = "topic";
const char* CONTEXTSTRING = "context";



class LogServer:noncopyable
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

	void find(string begin, string end)
	{
		_storager.Get(begin, end);
	}

	void onMessage(char* begin, char* end)
	{
		string timestamp, username, topic, context;
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
			else if (strcmp(begin, mid - begin, USERNAMESTRING))
			{
				username.assign(val, next);
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
		//printf("%s %d %s %d %s %d %s %d\n", timestamp, timestamplen, username, usernamelen, topic, topiclen, context, contextlen);
		Set(timestamp, username, topic, context);
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

	void Set(string timestamp, string username, string topic, string context)
	{
		message m = message(timestamp,  username,  topic,  context);
		_queue.push(std::move(m));
	}

	void Run()
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
			_storager.Set(_queue.front());
			count++;
			_queue.pop();
		}
	}

	HttpServer _server;
	storager _storager;
	std::thread* _thread;
	lock_free_queue<message> _queue;
};

