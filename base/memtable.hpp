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
#include "logfile.hpp"
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
		:_num(0), _logFilename(name + ".log0"), _indexFilename (name + ".index0"),_logfile(_logFilename){}

	~memtable(){}

	void Set(const message&);

	void Flush();
	void Close();
	void Restart();
	void WriteIndex();

	int Log(const message&);

	int Get(std::stringstream*, const message&, const message&);
	long Size(){return _logfile.WritePos();}
private:
	int _num;
	string _logFilename;
	string _indexFilename;
	Logfile _logfile;
	vector<int> _indexs;
	SkipList<message, int> _sortlist;
};

void memtable::Close()
{
	_logfile.Close();
	WriteIndex();
}


void memtable::Restart()
{
	char* timestamp;
	char* userid;
	char* topic;
	char* context;
	while (!_logfile.Eof())
	{
		int offset = _logfile.WritePos();
		_logfile.Read(timestamp, userid, topic, context);
		_logfile.ReadWrap();
		message m(timestamp, userid, topic,context);
		_sortlist.push_back(m, offset);
		_num++;
	}
}

void memtable::Flush()
{
	_logfile.Flush();
}

void memtable::WriteIndex()
{
	string filename = _indexFilename + "w";
	Logfile indexfile(filename,ios::trunc);
	indexfile.Write(_num);
	for (auto it = _sortlist.begin(); it != _sortlist.end(); it = it->next())
	{
		indexfile.Write(it->value);
	}
	for (auto it = _indexs.begin(); it != _indexs.end(); it++)
	{
		indexfile.Write(*it);
	}
	indexfile.Close();
	rename(filename.c_str(), _indexFilename.c_str());
}


int memtable::Get(std::stringstream* ss,const message& start_message, const message& end_message)
{
	auto start = _sortlist.find(start_message);
	auto end = _sortlist.find(end_message);
	int ret = 0;
	for (auto it = start; it != end; it = it->next())
	{
		ret++;
		*ss << "[" << it->key._timestamp << "] [" << it->key._username << "] [" << it->key._topic << "]: " << it->key._context << "\n";
	}
	return ret;
}



void memtable::Set(const message& m)
{
	int offset = static_cast<int>(_logfile.WritePos());
	Log(m);
	_indexs.push_back(offset);
	_sortlist.push_back(m, offset);
	_num++;
}


int memtable::Log(const message& m)
{
	_logfile.Write(m._timestamp, m._username, m._topic, m._context);
	_logfile.Writeline();
	_logfile.Flush();
	return 0;
}
