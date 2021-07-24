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

using std::string;
using std::vector;
using std::fstream;
using std::ios;
using std::shared_ptr;


static std::stringstream* _ss = new std::stringstream();


class Memkv
{
public:

	Memkv(string name)
		:_num(0), _logFilename(name + ".log0"), _indexFilename (name + ".index0"),_logfile(_logFilename){}

	~Memkv(){}

	void Set(const message&);

	void Flush();
	void Close();
	void Restart();
	void WriteIndex();

	int Log(const message&);

	int Get(Logfile*, string, string, string);
	int Get(Logfile*, string, string);
	long Size(){return _logfile.WritePos();}
private:
	int _num;
	string _logFilename;
	string _indexFilename;
	Logfile _logfile;
	vector<int> _indexs;
	SkipList<message, int> _timelist;
	SkipList<message, int> _userlist;
};

void Memkv::Close()
{
	_logfile.Close();
	WriteIndex();
}


void Memkv::Restart()
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
		_timelist.push_back(m, offset);
		_userlist.push_back(m, offset);
		_num++;
	}
}

void Memkv::Flush()
{
	_logfile.Flush();
}

void Memkv::WriteIndex()
{
	string filename = _indexFilename + "w";
	Logfile indexfile(filename,ios::trunc);
	indexfile.Write(_num);
	for (auto it = _timelist.begin(); it != _timelist.end(); it = it->next())
	{
		indexfile.Write(it->value);
	}
	for (auto it = _indexs.begin(); it != _indexs.end(); it++)
	{
		indexfile.Write(*it);
	}
	for (auto it = _userlist.begin(); it != _userlist.end(); it = it->next())
	{
		indexfile.Write(it->value);
	}
	indexfile.Close();
	rename(filename.c_str(), _indexFilename.c_str());
}


int Memkv::Get(Logfile* file,string start_time,string end_time)
{
	auto start = _timelist.find(message(start_time,"","",""));
	auto end = _timelist.find(message(end_time,"","",""));
	int ret = 0;
	for (auto it = start; it != end; it = it->next())
	{
		ret++;
		*file << "[" << it->key._timestamp << "] [" << it->key._username << "] [" << it->key._topic << "]: " << it->key._context << "\n";
	}
	file->Flush();
	return ret;
}


int Memkv::Get(Logfile* file,string userid, string start_time, string end_time)
{
	auto start = _userlist.find(message(start_time,userid,"",""));
	auto end = _userlist.find(message(end_time, userid, "", ""));
	int ret = 0;
	for (auto it = start; it != end; it = it->next())
	{
		ret++;
		*file << "[" << it->key._timestamp << "] [" <<it->key._username << "] ["<< it->key._topic << "]: " << it->key._context << "\n";
	}
	file->Flush();
	return ret;
}

void Memkv::Set(const message& m)
{
	int offset = static_cast<int>(_logfile.WritePos());
	_indexs.push_back(offset);
	Log(m);
	_timelist.push_back(m, offset);
	_userlist.push_back(m, offset);
	_num++;
}


int Memkv::Log(const message& m)
{
	std::cout << m._timestamp << " " << m._username << " " << m._topic << " " << m._context << std::endl;
	_logfile.Write(m._timestamp, m._username, m._topic, m._context);
	_logfile.Writeline();
	_logfile.Flush();
	return 0;
}
