#pragma once
#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>
#include <functional>
#include <sys/stat.h>
#include <algorithm>
#include <memory>
#include "iofile.hpp"
#include "skiplist.hpp"
#include "setting.hpp"
#include "message.hpp"
#include "noncopyable.hpp"

using std::string;
using std::vector;
using std::fstream;
using std::ios;
using std::shared_ptr;


static std::stringstream* _ss = new std::stringstream();


class memtable:noncopyable
{
public:

	memtable(string name)
		:_num(0), _name(Filepath + name),_dataFilename(_name + ".log"), _indexFilename (_name + ".index"),_offset(0),_max_time(""),_min_time("")
	{}

	~memtable(){}

	const string& name() { return _name; }
	void set(const message&);
	void flush();
	void close();
	void restart(const string&);
	void writeIndex();
	void writeData();
	int log(const message&);
	const string& max_time() { return _max_time; }
	const string& min_time() { return _min_time; }
	int get(std::stringstream*, const message&, const message&,int);
private:
	int _num;
	int _offset;
	string _name;
	string _max_time;
	string _min_time;
	string _dataFilename;
	string _indexFilename;
	SkipList<message, int32_t> _sortlist;
};

void memtable::close()
{
	writeIndex();
}


void memtable::restart(const string& file)
{
	string timestamp;
	string topic;
	string context;
	iofile tempfile(file);
	while (!tempfile.eof())
	{
		tempfile.Read(timestamp, topic, context);
		message m(timestamp, topic,context);
		_sortlist.push_back(m, 0);
		_num++;
	}
	tempfile.close();
}

void memtable::flush()
{
	writeData();
	writeIndex();
}

void memtable::writeData()
{
	iofile datafile(_dataFilename, ios::trunc);
	for (auto it = _sortlist.begin(); it != _sortlist.end(); it = it->next())
	{
		it->value = _offset;
		_offset += it->key.length();
		datafile.Write(it->key._timestamp,it->key._topic,it->key._context);
	}
	datafile.close();
}

void memtable::writeIndex()
{
	string filename = _indexFilename + "w";
	iofile indexfile(filename,ios::trunc);
	indexfile.Write(_num);
	for (auto it = _sortlist.begin(); it != _sortlist.end(); it = it->next())
	{
		indexfile.Write(it->value);
	}
	indexfile.Write(_offset);
	indexfile.close();
	rename(filename.c_str(), _indexFilename.c_str());
}


int memtable::get(std::stringstream* ss,const message& start_message, const message& end_message,int num)
{
	auto start = _sortlist.find(start_message);
	auto end = _sortlist.find(end_message);
	int ret = 0;
	for (auto it = start; it != end; it = it->next())
	{
		ret++;
		*ss << "[" << it->key._timestamp  << "] [" << it->key._topic << "]: " << it->key._context << "\n";
		if (ret == num) return ret;
	}
	return ret;
}

void memtable::set(const message& m)
{
	if (_max_time == "")
	{
		_max_time = _min_time = m._timestamp;
	}
	if (m._timestamp > _max_time) 
		_max_time = m._timestamp;
	if (m._timestamp < _min_time)
		_min_time = m._timestamp;
	_sortlist.push_back(m, 0);
	_num++;
}

int memtable::log(const message& m)
{
	//_logfile.Write(m._timestamp, m._topic, m._context);
	//_logfile.flush();
	return 0;
}
