#pragma once
#include <string>
#include <iostream>
#include "noncopyable.hpp"

using std::string;


//example:20210804-11:28:40-710313
struct Timestamp
{
	int day;
	int hour;
	int min;
	int sec;
	int microseconds;
	int hour_min_sec;
	int length = sizeof(int) * 3;
	Timestamp()
		:day(0), hour(0), min(0), sec(0), microseconds(0), hour_min_sec(0) {};
	Timestamp(const string& st)
		:day(0), hour(0), min(0), sec(0), microseconds(0), hour_min_sec(0)
	{
		for (int i = 0; i < 8 && i < st.length(); i++) day = day * 10 + (st[i] - '0');
		for (int i = 9; i < 11 && i < st.length(); i++) hour = hour * 10 + (st[i] - '0');
		for (int i = 12; i < 14 && i < st.length(); i++) min = min * 10 + (st[i] - '0');
		for (int i = 15; i < 17 && i < st.length(); i++) sec = sec * 10 + (st[i] - '0');
		for (int i = 18;  i < st.length(); i++) microseconds = microseconds * 10 + (st[i] - '0');
		hour_min_sec = hour * 10000 + min * 100 + sec;
	}

	Timestamp(const Timestamp& t)
		:day(t.day),hour(t.hour),min(t.min),sec(t.sec),microseconds(t.microseconds),hour_min_sec(t.hour_min_sec),length(t.length)
	{}


	void assign(int d, int hms, int micros)
	{
		day = d;
		hour_min_sec = hms;
		microseconds = micros;
		hour = hour_min_sec / 10000;
		min = (hour_min_sec % 10000) / 100;
		sec = hour_min_sec % 100;
	}

	bool operator > (const Timestamp& t1)
	{
		if (day != t1.day) return day > t1.day;
		if (hour_min_sec != t1.hour_min_sec) return hour_min_sec > t1.hour_min_sec;
		if (microseconds != t1.microseconds) return microseconds > t1.microseconds;
	}
};

bool operator > (const Timestamp& t,const Timestamp& t1)
{
	if (t.day != t1.day) return t.day > t1.day;
	if (t.hour_min_sec != t1.hour_min_sec) return t.hour_min_sec > t1.hour_min_sec;
	if (t.microseconds != t1.microseconds) return t.microseconds > t1.microseconds;
}

bool operator < (const Timestamp& t, const Timestamp& t1)
{
	if (t.day != t1.day) return t.day < t1.day;
	if (t.hour_min_sec != t1.hour_min_sec) return t.hour_min_sec < t1.hour_min_sec;
	if (t.microseconds != t1.microseconds) return t.microseconds < t1.microseconds;
}

std::ostream& operator << (std::ostream& os,const Timestamp& t)
{
	static char buf[32] = { 0 };
	snprintf(buf, sizeof(buf), "%08d-%02d:%02d:%02d-%06d",
		t.day,t.hour,t.min,t.sec,t.microseconds);
	os << buf;
}

class message
{
public:
	message()
	{
	}

	message(const string& timestamp, const string& topic, const string& context)
		:_timestamp(timestamp),
		_topic(topic),
		_context(context)
	{
		_len = sizeof(size_t) * 2 + _topic.length() + _timestamp.length + _context.length();
	}


	message(const Timestamp& timestamp, const string& topic, const string& context)
		:_timestamp(timestamp),
		_topic(topic),
		_context(context)
	{
		_len = sizeof(size_t) * 2 + _topic.length() + _timestamp.length + _context.length();
	}


	message(const message& m)
		:_timestamp(m._timestamp),
		_topic(m._topic),
		_context(m._context)
	{
		_len = sizeof(size_t) * 2 + _topic.length() + _timestamp.length + _context.length();
	}


	bool operator > (const message& m)
	{
		int ret = _topic.compare(m._topic);
		if (ret == 0)
			return _timestamp > m._timestamp;
		else
			return ret > 0;
	}

	~message()
	{

	}

	Timestamp _timestamp;
	string _topic;
	string _context;
	int length()
	{
		return _len;
	}
private:
	int _len;

};

bool operator > (const message& m1,const message& m2)
{
	int ret = m1._topic.compare(m2._topic);
	if (ret == 0)
		return m1._timestamp > m2._timestamp;
	else 
		return ret > 0;
}


//class message
//{
//public:
//	message() {}
//
//	message(const char* timestamp, const char* username, const char* topic, const char* context)
//		:_timestamp(new char[strlen(timestamp)]),
//		_username(new char[strlen(username)]),
//		_topic(new char[strlen(topic)]),
//		_context(new char[strlen(context)])
//	{
//		strcpy(_timestamp, timestamp);
//		strcpy(_username, username);
//		strcpy(_topic, topic);
//		strcpy(_context, context);
//	}
//
//	message(char* timestamp, int timestamplen, char* username, int usernamelen, char* topic, int topiclen, char* context, int contextlen)
//		:_timestamp(new char[timestamplen]),
//		_username(new char[usernamelen]),
//		_topic(new char[topiclen]),
//		_context(new char[contextlen])
//	{
//		if (timestamplen) memcpy(_timestamp, timestamp, timestamplen);
//		if (usernamelen) memcpy(_username, username, usernamelen);
//		if (topiclen) memcpy(_topic, topic, topiclen);
//		if (contextlen) memcpy(_context, context, contextlen);
//	}
//
//	message(const message& m)
//		:_timestamp(new char[strlen(m._timestamp)]),
//		_username(new char[strlen(m._username)]),
//		_topic(new char[strlen(m._topic)]),
//		_context(new char[strlen(m._context)])
//	{
//		strcpy(_timestamp, m._timestamp);
//		strcpy(_username, m._username);
//		strcpy(_topic, m._topic);
//		strcpy(_context, m._context);
//	}
//
//	~message()
//	{
//		delete[] _timestamp;
//		delete[] _username;
//		delete[] _topic;
//		delete[] _context;
//	}
//
//	bool operator > (const message& m)
//	{
//		int ret = 0;
//		ret = strcmp(_timestamp, m._timestamp);
//
//	}
//
//	char* _timestamp;
//	char* _username;
//	char* _topic;
//	char* _context;
//};
//class Key
//{
//public:
//
//	string _timestamp;
//	string _username;
//	string _topic;
//	string _user_time_topic_key;
//	string _time_user_topic_key;
//	Key()
//	{
//	}
//
//	Key(char* timestamp, char* userid, char* topic)
//		:_timestamp(timestamp), _username(userid), _topic(topic)
//	{
//		*_ss << _username << "-" << _timestamp << "-" << _topic;
//		_user_time_topic_key = _ss->str();
//		_ss->str("");
//		*_ss << _timestamp << "-" << _username << "-" << _topic;
//		_time_user_topic_key = _ss->str();
//		_ss->str("");
//	}
//
//	Key(string timestamp, string userid, string topic)
//		:_timestamp(timestamp), _username(userid), _topic(topic)
//	{
//		*_ss << _username << "-" << _timestamp << "-" << _topic;
//		_user_time_topic_key = _ss->str();
//		_ss->str("");
//		*_ss << _timestamp << "-" << _username << "-" << _topic;
//		_time_user_topic_key = _ss->str();
//		_ss->str("");
//	}
//
//	~Key()
//	{
//	}
//};
//
