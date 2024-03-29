#pragma once
#include <string>
#include <algorithm>
#include <iostream>
#include "../base/buffer.hpp"


const char CRLF[] = "\r\n";
const char Http_Header_End[] = "\r\n\r\n";

using std::string;

char* get_http_header_end(char* begin, char* end)
{
	char* ret = std::search(begin, end, Http_Header_End, Http_Header_End + 4);
}

enum HttpMethod
{
	ERROR = 0,
	GET = 1,
	POST = 2,
	PUT = 3,
	OPTIONS = 4
};

class httpHeader
{
public:
	httpHeader(char* begin, char* end)
		:_begin(begin),_end(end),searchkey(""),num(100000)
	{
		getMethod();
		getUrl();
		getVersion();
		getAttributes();
	}
	httpHeader()
		:searchkey(""), num(100000)
	{}

	~httpHeader(){}
	HttpMethod method(){return _method;}
	int num;
	int datalength = 0;
	string version;
	string url;
	string searchkey;
	string topic;
	string begin;
	string end;
private:
	void getAttributes();
	void getUrl();
	void getMethod();
	void getVersion();

	char* _begin;
	char* _end;
	HttpMethod _method;
};


void httpHeader::getMethod()
{
	char* empty = std::find(_begin, _end, ' ');
	string Method(_begin, empty);
	if (Method == "GET")
	{
		_method = HttpMethod::GET;
	}
	else if (Method == "POST")
	{
		_method = HttpMethod::POST;
	}
	else if (Method == "PUT")
	{
		_method = HttpMethod::PUT;
	}
	else if (Method == "OPTIONS")
	{
		_method = HttpMethod::OPTIONS;
	}
	else
	{
		_method = HttpMethod::ERROR;
	}
	_begin = empty + 1;
}

void httpHeader::getUrl()
{
	char* space = std::find(_begin, _end, ' ');
	char* next = std::find(_begin, space, '?');
	url = string(_begin, next);
	while (next != space)
	{
		_begin = next + 1;
		next = std::find(_begin, space, '=');
		string key(_begin, next);
		_begin = next + 1;
		next = std::find(_begin, space, '&');
		if (key == "topic")
		{
			topic = string(_begin, next);
		}
		else if (key == "begin")
		{
			begin = string(_begin, next);
		}
		else if (key == "end")
		{
			end = string(_begin, next);
			end[end.length() - 1]++;
		}
		else if (key == "num")
		{
			num = std::stoi(string(_begin, next));
		}
		else if (key == "key")
		{
			searchkey = string(_begin, next);
		}
	}
	_begin = next + 1;
}

void httpHeader::getVersion()
{
	char* next = std::search(_begin, _end, CRLF, CRLF + 2);
	version = string(_begin, next);
	_begin = next + 2;
}

void httpHeader::getAttributes()
{
	char* next = std::find(_begin, _end, ':');
	for (; next != _end; next = std::find(_begin, _end, ':'))
	{
		string key(_begin, next);
		_begin = next + 2;
		next = std::search(_begin, _end, CRLF, CRLF + 2);
		if (key == "Content-Length")
		{
			string val(_begin, next);
			datalength = std::stoi(val);
		}
		_begin = next + 2;
	}
}