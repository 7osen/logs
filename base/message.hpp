#pragma once
#include <string>
#include "noncopyable.hpp"

using std::string;


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
		_len = sizeof(size_t) * 3 + _topic.length() + _timestamp.length() + _context.length();
	}


	message(const message& m)
		:_timestamp(m._timestamp),
		_topic(m._topic),
		_context(m._context)
	{
		_len = sizeof(size_t) * 3 + _topic.length() + _timestamp.length() + _context.length();
	}


	bool operator > (const message& m)
	{
		if (_topic != m._topic) return _topic > m._topic;
		if (_timestamp != m._timestamp) return _timestamp > m._timestamp;
		return _context > m._context;
	}

	~message()
	{

	}

	string _timestamp;
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
	if (m1._topic != m2._topic) return m1._topic > m2._topic;
	if (m1._timestamp != m2._timestamp) return m1._timestamp > m2._timestamp;
	return m1._context > m2._context;
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
