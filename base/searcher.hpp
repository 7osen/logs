#pragma once
#include "storager.hpp"
#include "../net/HttpServer.hpp"

struct searchRequest
{
	Connect* connect;
	httpHeader* header;
		searcher(Connect* conn, httpHeader* h)
	{
		connect = conn;
		header = h;
	}
};

class searcher
{
public:
	searcher(storager* st)
		:_storager(st),_queue()
	{

	}

	void query(Connect* conn, httpHeader* header)
	{
		_queue.push(searchRequest(conn, header));
		_s.wakeup();
	}

	void start()
	{
		_searcherThread = new thread(&searcher::run, this);
		_searcherThread->detach();
	}

	~searcher()
	{

	}

private:
	void run()
	{
		std::stringstream* ss = new std::stringstream();
		for (;;)
		{
			_s.wait();
			searchRequest request = _queue.front();
			TimeCount t;
			t.Update();
			_storager->get(ss, request.header->topic, request.header->begin, request.header->end, request.header->num, request.header->searchkey);
			st = ss->str();
			request.connect->Write(st);
			delete ss;
			std::cout << t.getMircoSec() << std::endl;
			_queue.pop();
			ss->str("");
		}
	}

	storager* _storager;
	thread* _searcherThread;
	mq<searchRequest> _queue;
	semaphore _s;
};

