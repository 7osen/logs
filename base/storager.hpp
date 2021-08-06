#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include "timecount.hpp"
#include "memtable.hpp"
#include "mq.hpp"
#include "metadata.hpp"
using std::vector;
using std::mutex;
using std::thread;
using std::to_string;


class storager:noncopyable
{
public:
	storager(const string& filename = "log")
		:_filenum(1),_tempnum(0), _tempnow(0),_filename(Filepath + filename), _lastmems(nullptr), _tables(100)
	{
		_mems = createMemtable();
		_metadata.restart();
		_mems->restart(_filename + ".temp");
		_tempnum++;
		_tempfile = new iofile(_filename + ".temp");
	}
	void start();
	void set(const message&);
	int get(std::stringstream* ss, const string& topic, const string& start_time,const string& end_time, int num, const string& key);
	virtual ~storager() {}
protected:
	void flush();
	void startflush();
	void resetmem();
	void roll();

	virtual void memchange(memtable*) = 0;
	virtual memtable* createMemtable() = 0;
	virtual int getFromFile(matcher*, logfile*, const message&, const message&, int num) = 0;

	
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
};


void storager::start()
{
	
	startflush();
}

void storager::startflush()
{
	_thread = new thread(std::bind(&storager::flush, this));
	_thread->detach();
}

void storager::flush()
{
	while (1)
	{
		_sem.wait();
		memtable* table = _tables.front();
		table->flush();
	
		memchange(table);
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
		roll();
	}
}

void storager::roll()
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

int storager::get(std::stringstream* ss, const string& topic, const string& start_time, const string& end_time, int num,const string& key = "")
{
	matcher match(key);
	message start(start_time, topic, "");
	message end(end_time , topic, "");
	match.setStringstream(ss);
	int ret = 0;
	if (num > 100000) num = 100000;
	std::lock_guard<mutex> lock(_findmutex);
	for (auto it = _metadata.begin(); it != _metadata.end(); it++)
		if ((*it)->min_time() > (end_time) || (*it)->max_time() < start_time) 
			continue;
		else 
		{
			ret += getFromFile(&match, *it, start, end, num - ret);
			if (ret >= num || match.size() <= 0) return ret;
		}
	{
		std::lock_guard<mutex> lock(_insertmutex);
		if (ret >= num || match.size() <= 0) return ret;
		if (_lastmems != nullptr) ret += _lastmems->get(&match, start, end, num - ret);
		if (ret >= num || match.size() <= 0) return ret;
		ret += _mems->get(&match, start, end, num - ret);
	}
}



void storager::set(const message& m)
{
	_mems->set(m);
	_tempfile->Write(m._timestamp, m._topic, m._context);
	_tempfile->flush();
	if (_tempfile->writePos() > MaxFileSize)
	{
		resetmem();
	}
}

void storager::resetmem()
{
	_tempfile->close();
	rename(_tempfile->name().c_str(), (_filename + ".temp" + std::to_string(_tempnum - 1)).c_str());
	_tempnum++;
	_tempfile = new iofile(_filename + ".temp");
	_tables.push(_mems);
	{
		std::lock_guard<mutex> lock(_insertmutex);
		_lastmems = _mems;
		_mems = createMemtable();
	}
	_sem.wakeup();
}