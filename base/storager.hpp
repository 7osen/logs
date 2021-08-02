#pragma once
#include <vector>
#include <thread>
#include "timecount.hpp"
#include "memtable.hpp"
#include "mq.hpp"
#include "file_manager.hpp"
using std::vector;
using std::thread;
using std::to_string;

class storager:noncopyable
{
public:
	storager(string filename = "log")
		:_filenum(1),_tempnum(0), _tempnow(0),_filename(Filepath + filename),_mems(new memtable(getTimenow()))
	{
		restart();
		_tempnum++;
		_tempfile = new iofile(_filename + ".temp");
		startflush();
	}
	~storager(){}
	void startflush();
	void restart();
	void set(const message&);
	void flush();
	int get(std::stringstream*,const string&, const string&, const string&,int num);
	
private:
	void roll();
	int getFromFile(std::stringstream*,const string&,const string&, const message&, const message&,int num);
	int find(iofile*, iofile*,const message&, int);
	int _filenum;
	int _tempnow;
	int _tempnum;
	iofile* _tempfile;
	string _filename;
	memtable* _mems;
	semaphore _sem;
	thread* _thread;
	logfileManager _lfmanager;
	mq<memtable*> _tables;
};


void storager::restart()
{
	_lfmanager.restart();
	_mems->restart(_filename + ".temp");
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
		_lfmanager.push_back(new logfile(table->name(), table->min_time(), table->max_time()));
		delete table;
		_tables.pop();
		_tempnum--;
		roll();
	}
}

void storager::roll()
{
	int j = 0;
	for (int i = 0; i < _tempnum; i++)
	{
		for (; j < _tempnow; j++)
		{
			struct stat file;
			if (stat((_filename + ".temp" + to_string(j)).c_str(), &file) == 0)
			{
				rename((_filename + ".temp" + to_string(j)).c_str(), (_filename + ".temp" + to_string(i)).c_str());
				break;
			}
		}
	}
}

int storager::find(iofile* log, iofile* index,const message& v,int num)
{
	message val;
	int l = 1, r = num, mid, offset;
	index->setReadPos(sizeof(int) * num);
	index->Read(offset);
	log->setReadPos(offset);
	log->Read(val._timestamp,val._topic);
	if (v > val) return (r + 1) * sizeof(int);
	for (; l < r;)
	{
		mid = (l + r) >> 1;
		index->setReadPos(sizeof(int) * mid);
		index->Read(offset);
		log->setReadPos(offset);
		log->Read(val._timestamp, val._topic);
		if (v > val)
		{
			l = mid + 1;
		}
		else r = mid;
	}
	return r * sizeof(int) ;
}


int storager::getFromFile(std::stringstream* ss,const string& logFilename,const string& indexFilename, const message& start_time,const message& end_time,int num)
{
	iofile log(logFilename);
	iofile index(indexFilename);
	int n = 0,offset = 0,ret = 0;
	index.Read(n);
	int begin = find(&log, &index,start_time, n);
	int end = find(&log, &index, end_time, n);
	index.setReadPos(begin);
	index.Read(begin);
	index.setReadPos(end);
	index.Read(end);
	string timestamp;
	string topic;
	string context;
	log.setReadPos(begin);
	for (; log.readPos() < end && !log.eof();)
	{
		if (ret == num) return ret;
		ret++;
		log.Read(timestamp, topic, context);
		*ss << "[" << timestamp << "] [" << topic << "]: " << context << "\n";
	}
	return ret;
}


int storager::get(std::stringstream* ss,const string& topic,const string& start_time,const string& end_time,int num)
{
	message start(start_time, topic, "");
	message end(end_time + "?", topic, "");
	int ret = 0;
	ret = _mems->get(ss, start, end,num);
	for (auto it = _lfmanager.begin(); it != _lfmanager.end(); it++)
	{
		if (ret >= num) return ret;
		ret += getFromFile(ss, (*it)->datafilename(), (*it)->indexfilename(), start, end,num - ret);
	}
	return ret;
}

void storager::set(const message& m)
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
		_tables.push(_mems);
		_mems = new memtable(getTimenow());
		_sem.wakeup();
	}
}
