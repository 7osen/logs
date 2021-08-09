#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include "timecount.hpp"
#include "memtable.hpp"
#include "mq.hpp"
#include "metadata.hpp"
#include "threadpool.hpp"
#include "../net/HttpHeader.hpp"
using std::vector;
using std::mutex;
using std::thread;
using std::to_string;


class database:noncopyable
{
public:
	database(const string& filename = "log")
		:_filenum(1), _tempnum(0), _tempnow(0), _filename(Filepath + filename), _lastmems(nullptr), _tables(100), _threadPool(4)
	{
		
	}
	void start();
	void set(const message&);
	int get(std::stringstream* ss, shared_ptr<httpHeader>);
	virtual ~database() {}
protected:
	void flush();
	void startflush();
	void resetmem();
	void clearTemp();


	virtual void restart() = 0;
	virtual void push_cache(memtable*) = 0;
	virtual memtable* createMemtable() = 0;
	virtual int get_from_file(matcher*, logfile*, const message&, const message&, int num) = 0;

	
	int _filenum;
	int _tempnow;
	int _tempnum;
	iofile* _tempfile;
	memtable* _mems;
	memtable* _lastmems;
	string _filename;
	semaphore _sem;
	thread* _thread;
	mutex _findmutex;
	mutex _insertmutex;
	metadata _metadata;
	mq<memtable*> _tables;
	ThreadPool _threadPool;
};


void database::start()
{
	_mems = createMemtable();
	_metadata.restart();
	restart();
	_mems->restart(_filename + ".temp");
	_tempnum++;
	_tempfile = new iofile(_filename + ".temp");
	startflush();
}

void database::startflush()
{
	//_thread = new thread(std::bind(&database::flush, this));
	//_thread->detach();
}

void database::flush()
{
	memtable* table = _tables.front();
	table->flush();
	push_cache(table);
	{
		std::lock_guard<mutex> lock(_findmutex);
		_metadata.push_back(new logfile(table->name(), table->min_time(), table->max_time()));
		{
			if (_lastmems == table) 
			_lastmems = nullptr;
		}
	}
	delete table;
	_tables.pop();
	_tempnum--;
	clearTemp();
}

void database::clearTemp()
{
	remove((_filename + ".temp0" ) .c_str());
	for (int i = 1; i < _tempnum; i++)
	{
			struct stat file;
			if (stat((_filename + ".temp" + to_string(i)).c_str(), &file) == 0)
			{
				rename((_filename + ".temp" + to_string(i)).c_str(), (_filename + ".temp" + to_string(i - 1)).c_str());
				break;
			}
	}
}

int database::get(std::stringstream* ss, shared_ptr<httpHeader> header)
{
	message start(header->begin, header->topic, "");
	message end(header->end, header->topic, "");
	int num = header->num;
	if (num > MaxFindNum) num = MaxFindNum;
	matcher match(header->searchkey,num);
	int ret = 0;
	match.setStringstream(ss);
	std::lock_guard<mutex> lock(_findmutex);
	if (header->searchkey == "")
	{
		for (auto it = _metadata.begin(); it != _metadata.end(); it++)
			if ((*it)->min_time() > end._timestamp || (*it)->max_time() < start._timestamp)
				continue;
			else
			{
				ret += get_from_file(&match, *it, start, end, num - ret);
				if (ret >= num || match.size() <= 0) return ret;
			}
	}
	else
	{
		std::vector<std::future<int>> results;
		for (auto it = _metadata.begin(); it != _metadata.end(); it++)
			if ((*it)->min_time() > end._timestamp || (*it)->max_time() < start._timestamp)
				continue;
			else
			{
				results.emplace_back(_threadPool.enqueue(std::bind(&database::get_from_file, this, &match, *it, start, end, num - ret)));
			}
		for (auto&& result : results)
		{
				ret += result.get();
		}
	}
	{
		std::lock_guard<mutex> lock(_insertmutex);
		if (ret >= num || match.size() <= 0) return ret;
		if (_lastmems != nullptr) ret += _lastmems->get(&match, start, end, num - ret);
		if (ret >= num || match.size() <= 0) return ret;
		ret += _mems->get(&match, start, end, num - ret);
	}
}



void database::set(const message& m)
{
	_mems->set(m);
	_tempfile->Write(m._timestamp, m._topic, m._context);
	_tempfile->flush();
	if (_tempfile->writePos() > MaxFileSize)
	{
		_tempfile->close();
		rename(_tempfile->name().c_str(), (_filename + ".temp" + std::to_string(_tempnum - 1)).c_str());
		_tempnum++;
		_tempfile = new iofile(_filename + ".temp");
		resetmem();
	}
}

void database::resetmem()
{
	_tables.push(_mems);
	{
		std::lock_guard<mutex> lock(_insertmutex);
		_lastmems = _mems;
		_mems = createMemtable();
	}
	_threadPool.enqueue(std::bind(&database::flush,this));
}