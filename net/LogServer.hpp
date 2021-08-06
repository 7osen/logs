#pragma once
#include "../base/plaintableStorager.hpp"
#include "../base/blocktableStorager.hpp"
#include "../base/mq.hpp"
#include "../base//timecount.hpp"
#include "../base/writer.hpp"
#include "../base/reader.hpp"
#include "HttpServer.hpp"
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

		_storager = new plaintableStorager();
		_writer = new writer(_storager);
		_reader = new reader(_storager);
		_writer->start();
		_reader->start();
		_storager->start();
		_server.setPostCallBack(std::bind(&LogServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
		_server.setGetCallBack(std::bind(&LogServer::find, this, std::placeholders::_1, std::placeholders::_2));
	}

	~LogServer()
	{

		delete _storager;
	}

	void start()
	{
		_server.start();
	}

	void close()
	{
		_server.close();
	}

private:
	void find(Connect* conn, shared_ptr<httpHeader> header)
	{
		_reader->query(conn, header);
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
		_writer->set(timestamp, topic, context);
	}
	bool strcmp(char* ch, int len, const char* src)
	{
		for (int i = 0; i < len; i++)
		{
			if (ch[i] != src[i]) return false;
		}
		return true;
	}

	storager* _storager;
	writer* _writer;
	reader* _reader;
	std::thread* _writerThread;
	HttpServer _server;
};

