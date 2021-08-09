#pragma once
#include <memory>
#include "database.hpp"
#include "../net/HttpServer.hpp"
using std::shared_ptr;

struct searchRequest
{
	Connect* connect;
	shared_ptr<httpHeader> header;
	searchRequest()
	{}
	searchRequest(Connect* conn, shared_ptr<httpHeader> h)
	{
		connect = conn;
		header = h;
	}
};

class reader
{
public:
	reader(database* st)
		:_storager(st),_queue(10)
	{}

	void query(Connect* conn, shared_ptr<httpHeader> header)
	{
		_queue.push(searchRequest(conn, header));
		_s.wakeup();
	}

	void start()
	{
		_searcherThread = new thread(&reader::run, this);
		_searcherThread->detach();
	}

	~reader()
	{
	}

private:
	void run()
	{
		std::stringstream ss;
		std::stringstream ssparam;
		for (;;)
		{
			_s.wait();
			searchRequest request = _queue.front();
			TimeCount t;
			t.Update();
			_storager->get(&ssparam, request.header);
			ss << response << ssparam.str().length() << "\r\n\r\n" << ssparam.str();
			request.connect->Write(ss.str());
			_queue.pop();
			ss.str("");
			ssparam.str("");
		}
	}

	database* _storager;
	thread* _searcherThread;
	mq<searchRequest> _queue;
	semaphore _s;
};

