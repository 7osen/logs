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
		int l = st.length();
		for (int i = 0; i < 8 && i < l; i++) day = day * 10 + (st[i] - '0');
		for (int i = 9; i < 11 && i < l; i++) hour = hour * 10 + (st[i] - '0');
		for (int i = 12; i < 14 && i < l; i++) min = min * 10 + (st[i] - '0');
		for (int i = 15; i < 17 && i < l; i++) sec = sec * 10 + (st[i] - '0');
		for (int i = 18;  i < l; i++) microseconds = microseconds * 10 + (st[i] - '0');
		hour_min_sec = hour * 10000 + min * 100 + sec;
	}

	Timestamp(const Timestamp& t)
		:	day(t.day),
			hour(t.hour),
			min(t.min),
			sec(t.sec),
			microseconds(t.microseconds),
			hour_min_sec(t.hour_min_sec),
			length(t.length)
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
	message(){}
	~message() {}
	int length() { return _len; }
	
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


	Timestamp _timestamp;
	string _topic;
	string _context;

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