#pragma once
#include <vector>
#include <thread>
#include "memtable.hpp"
#include "lock_free_queue.hpp"
#include "semaphore.hpp"
using std::vector;
using std::thread;
using std::mutex;
using std::to_string;

class storager:noncopyable
{
public:
	storager(string filename = "log")
		:_filenum(1),_tempnum(0), _tempnow(0),_filename(Filepath + filename), _mems(new memtable(_filename))
	{
		_logFileNames.emplace_back(_filename + ".log0");
		_indexFileNames.emplace_back(_filename + ".index0");
		restart();
		_tempnum++;
		_tempfile = new logfile(_filename + ".temp");
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
	void rolltemp();
	void getFileNum();
	bool file_exists(const std::string& name);
	int getFromFile(std::stringstream*,const string&,const string&, const message&, const message&,int num);
	int find(logfile*, logfile*,const message&, int);

	int _filenum;
	int _tempnow;
	int _tempnum;
	logfile* _tempfile;
	string _filename;
	memtable* _mems;
	thread* _thread;
	semaphore _semaphore;
	vector<string> _logFileNames;
	vector<string> _indexFileNames;
	lock_free_queue<memtable*> _tables;
};

bool storager::file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

void storager::restart()
{
	getFileNum();
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
		_semaphore.wait();
		memtable* table = _tables.front();
		table->flush();
		_tables.pop();
		_tempnum--;
		roll();
	}

}

void storager::roll()
{

	if (_filenum < MaxFileNum)
	{
		_filenum++;
		_logFileNames.push_back(_filename + ".log" + std::to_string(_filenum - 1));
		_indexFileNames.push_back(_filename + ".index" + std::to_string(_filenum - 1));
	}
	else
	{
		remove(_logFileNames[_filenum - 1].c_str());
		remove(_indexFileNames[_filenum - 1].c_str());
	}
	string newfilename = _logFileNames[_filenum - 1];
	string newindexname = _indexFileNames[_filenum - 1];
	string oldfilename;
	string oldindexname;
	for (int i = _filenum - 2; i >= 0; i--)
	{
		oldfilename = _logFileNames[i];
		oldindexname = _indexFileNames[i];
		rename(oldfilename.c_str(), newfilename.c_str());
		rename(oldindexname.c_str(), newindexname.c_str());
		newfilename = oldfilename;
		newindexname = oldindexname;
	}
	int j = 0;
	for (int i = 0; i < _tempnum; i++)
	{
		for (; j < _tempnow; j++)
			if (file_exists(_filename + ".temp" + to_string(j)))
			{
				rename((_filename + ".temp" + to_string(j)).c_str(), (_filename + ".temp" + to_string(i)).c_str());
				break;
			}
	}
}

int storager::find(logfile* log, logfile* index,const message& v,int num)
{
	message val;
	int l = 1, r = num, mid, offset;
	index->setReadPos(sizeof(int) * num);
	index->Read(offset);
	log->setReadPos(offset);
	log->Read(val._timestamp,val._topic,val._context);
	if (v > val) return (r + 1) * sizeof(int);
	for (; l < r;)
	{
		mid = (l + r) >> 1;
		index->setReadPos(sizeof(int) * mid);
		index->Read(offset);
		log->setReadPos(offset);
		log->Read(val._timestamp, val._topic, val._context);
		if (val > v)
		{
			r = mid;
		}
		else l = mid + 1;
	}
	return r * sizeof(int) ;
}


int storager::getFromFile(std::stringstream* ss,const string& logFilename,const string& indexFilename, const message& start_time,const message& end_time,int num)
{
	logfile log(logFilename);
	logfile index(indexFilename);
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
	for (int i = 1; i < _logFileNames.size(); i++)
	{
		if (ret >= num) return ret;
		ret += getFromFile(ss, _logFileNames[i], _indexFileNames[i], start, end,num - ret);
	}
	return ret;
}

void storager::getFileNum()
{
	string name;
	for (int i = 0; i < MaxFileNum; i++)
	{
		name = _filename + ".log" + std::to_string(i);
		if (file_exists(name))
		{
			_logFileNames.emplace_back(name);
			_indexFileNames.emplace_back(_filename + ".index" + std::to_string(i));
			_filenum = i + 1;
		}
		else break;
	}
}

void storager::set(const message& m)
{
	_mems->set(m);
	_tempfile->Write(m._timestamp, m._topic, m._context);
	_tempfile->flush();
	if (_tempfile->writePos() > MaxFileSize)
	{
		_tables.push(_mems);
		_mems = new memtable(_filename);
		_tempfile->close();
		rename(_tempfile->name().c_str(), (_filename + ".temp" + std::to_string(_tempnum - 1)).c_str());
		_tempnum++;
		_tempfile = new logfile(_filename + ".temp");
		_semaphore.wakeup();
	}
}
