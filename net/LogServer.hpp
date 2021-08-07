#pragma once
#include "../base/plainDatabase.hpp"
#include "../base/blockDatabase.hpp"
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

		_storager = new blockDatabase();
		_writer = new writer(_storager);
		_reader = new reader(_storager);
		_writer->start();
		_reader->start();
		_storager->start();
		_server.setPostCallBack(std::bind(&LogServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
		_server.setGetCallBack(std::bind(&LogServer::find, this, std::placeholders::_1, std::placeholders::_2));
	}

	~LogServer() {delete _storager;}
	void start(){_server.start();}
	void close(){_server.close();}

private:
	void onMessage(char* begin, char* end);
	void find(Connect* conn, shared_ptr<httpHeader> header) {_reader->query(conn, header);}
	//bool strcmp(char* ch, int len, const char* src)
	//{
	//	for (int i = 0; i < len; i++)
	//	{
	//		if (ch[i] != src[i]) return false;
	//	}
	//	return true;
	//}
	database* _storager;
	writer* _writer;
	reader* _reader;
	std::thread* _writerThread;
	HttpServer _server;
};

void LogServer::onMessage(char* begin, char* end)
{
	string timestamp, topic, context;
	char* mid = std::find(begin, end, '=');
	for (; mid != end; mid = std::find(begin, end, '='))
	{
		char* next = std::find(mid + 1, end, '&');
		char* val = mid + 1;
		int vallen = next - val;
		if (memcmp(begin, TIMESTRING, mid - begin) == 0)
		{
			timestamp.assign(val, next);
		}
		else if (memcmp(begin, TOPICSTRING, mid - begin) == 0)
		{
			topic.assign(val, next);
		}
		else if (memcmp(begin, CONTEXTSTRING, mid - begin) == 0)
		{
			context.assign(val, next);
		}
		begin = next + 1;
	}
	_writer->set(timestamp, topic, context);
}