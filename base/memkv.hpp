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

using std::string;
using std::vector;
using std::fstream;
using std::ios;
using std::shared_ptr;


static std::stringstream* _ss = new std::stringstream();

class Key
{
public:

	string _timestamp;
	string _userid;
	string _topic;
	string _user_time_topic_key;
	string _time_user_topic_key;
	Key()
	{
	}

	Key(char* timestamp, char* userid, char* topic)
		:_timestamp(timestamp),_userid(userid),_topic(topic)
	{
		*_ss << _userid << "-" << _timestamp << "-" << _topic;
		_user_time_topic_key = _ss->str();
		_ss->str("");	
		*_ss <<  _timestamp << "-" << _userid << "-" << _topic;
		_time_user_topic_key = _ss->str();
		_ss->str("");
	}

	Key(string timestamp, string userid, string topic)
		:_timestamp(timestamp), _userid(userid), _topic(topic)
	{
		*_ss << _userid << "-" << _timestamp << "-" << _topic;
		_user_time_topic_key = _ss->str();
		_ss->str("");
		*_ss << _timestamp << "-" << _userid << "-" << _topic;
		_time_user_topic_key = _ss->str();
		_ss->str("");
	}

	~Key()
	{
	}
};




class Value
{
public:
	int _offset;
	string _context;
	Value(string context, int offset)
		:_context(context), _offset(offset){}
	~Value(){}
private:

};


class message
{
public:
	message(){}

	message(const char* timestamp,const char* userid,const char* topic,const char* context)
		:_timestamp(new char[strlen(timestamp)]),
		_userid(new char[strlen(userid)]),
		_topic(new char[strlen(topic)]),
		_context(new char[strlen(context)])
	{
		strcpy(_timestamp, timestamp);
		strcpy(_userid, userid);
		strcpy(_topic, topic);
		strcpy(_context, context);
	}

	message(char* timestamp, int timestamplen,char* userid,int useridlen, char* topic,int topiclen, char* context,int contextlen)
		:_timestamp(new char[timestamplen]),
		_userid(new char[useridlen]),
		_topic(new char[topiclen]),
		_context(new char[contextlen])
	{
		strcpy(_timestamp, timestamp);
		strcpy(_userid, userid);
		strcpy(_topic, topic);
		strcpy(_context, context);
	}

	message(const message& m)
		:_timestamp(new char[strlen(m._timestamp)]),
		_userid(new char[strlen(m._userid)]),
		_topic(new char[strlen(m._topic)]),
		_context(new char[strlen(m._context)])
	{
		strcpy(_timestamp, m._timestamp);
		strcpy(_userid, m._userid);
		strcpy(_topic, m._topic);
		strcpy(_context, m._context);
	}

	~message()
	{
		delete[] _timestamp;
		delete[] _userid;
		delete[] _topic;
		delete[] _context;
	}

	bool operator > (const message& m)
	{
		int ret = 0;
		ret = strcmp(_timestamp, m._timestamp);

	}

	char* _timestamp;
	char* _userid;
	char* _topic;
	char* _context;
};


class Memkv
{
public:

	Memkv(string name)
		:_num(0), _logFilename(name + ".log0"), _indexFilename (name + ".index0"),_logfile(_logFilename){}

	~Memkv(){}

	void Set(Key, string);
	void Set(const message&);

	void Flush();
	void Close();
	void Restart();
	void WriteIndex();

	int Log(Key, string);
	int Log(const message&);

	int Get(Logfile*, string, string, string);
	int Get(Logfile*, string, string);
	long Size(){return _logfile.WritePos();}
private:
	int _num;
	string _logFilename;
	string _indexFilename;
	Logfile _logfile;
	SkipList<string, shared_ptr<Value> > _timelist;
	SkipList<string, shared_ptr<Value> > _userlist;
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
		Key k(timestamp, userid, topic);
		shared_ptr<Value> v(new Value(context, offset));
		_timelist.push_back(k._time_user_topic_key, v);
		_userlist.push_back(k._user_time_topic_key, v);
		_num++;
	}
}

int Memkv::Log(Key k, string context)
{
	_logfile.Write(k._timestamp,k._userid,k._topic, context);
	_logfile.Writeline();
	_logfile.Flush();
	return 0;
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
		indexfile.Write(it->value->_offset);
	}
	for (auto it = _userlist.begin(); it != _userlist.end(); it = it->next())
	{
		indexfile.Write(it->value->_offset);
	}
	indexfile.Close();
	rename(filename.c_str(), _indexFilename.c_str());
}


int Memkv::Get(Logfile* file,string start_time,string end_time)
{
	auto start = _timelist.find(start_time);
	auto end = _timelist.find(end_time);
	int ret = 0;
	for (auto it = start; it != end; it = it->next())
	{
		ret++;
		*file << it->key << ": "<< it->value->_context << "\n";
	}
	file->Flush();
	return ret;
}


int Memkv::Get(Logfile* file,string userid, string start_time, string end_time)
{
	auto start = _timelist.find(userid + "-" + start_time);
	auto end = _timelist.find(userid + "-" + end_time);
	int ret = 0;
	for (auto it = start; it != end; it = it->next())
	{
		ret++;
		*file << it->key << ": "<< it->value->_context << "\n";
	}
	file->Flush();
	return ret;
}

void Memkv::Set(const message& m)
{
	int offset = static_cast<int>(_logfile.WritePos());
	Log(m);
}

void Memkv::Set(Key key, string context)
{
	int offset = static_cast<int>(_logfile.WritePos());
	Log(key, context);
	shared_ptr<Value> v(new Value(context, offset));
	_timelist.push_back(key._time_user_topic_key, v);
	_userlist.push_back(key._user_time_topic_key, v);
	_num++;
}

int Memkv::Log(const message& m)
{
	std::cout << m._timestamp << std::endl;
}