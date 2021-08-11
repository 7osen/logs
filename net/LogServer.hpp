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

		_db = new blockDatabase();
		_writer = new writer(_db);
		_reader = new reader(_db);
		_server.setPostCallBack(std::bind(&LogServer::write, this, std::placeholders::_1, std::placeholders::_2));
		_server.setGetCallBack(std::bind(&LogServer::find, this, std::placeholders::_1, std::placeholders::_2));
	}

	~LogServer() {delete _db;}
	void start();
	void close(){_server.close();}

private:
	void write(char* begin, char* end);
	void find(Connect* conn, shared_ptr<httpHeader> header) {_reader->query(conn, header);}

	database* _db;
	writer* _writer;
	reader* _reader;
	std::thread* _writerThread;
	HttpServer _server;
};


void LogServer::start()
{
	_writer->start();
	_reader->start();
	_db->start();
	_server.start();
}

void LogServer::write(char* begin, char* end)
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