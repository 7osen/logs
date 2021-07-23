#pragma once
#include <string>
#include <algorithm>
#include <map>
#include <iostream>
#include "../base/buffer.hpp"


const char CRLF[] = "\r\n";
const char Http_Header_End[] = "\r\n\r\n";

using std::string;
using std::map;

char* get_http_header_end(char* begin, char* end)
{
	char* ret = std::search(begin, end, Http_Header_End, Http_Header_End + 4);
}

enum HttpMethod
{
	ERROR = 0,
	GET = 1,
	POST = 2,
	PUT = 3
};

class httpHeader
{
public:
	httpHeader(char* begin, char* end)
		:_begin(begin),_end(end)
	{
		getMethod();
		getUrl();
		getVersion();
		getAttributes();
	}

	~httpHeader()
	{}

	void print()
	{
		for (auto attr : _attributes)
		{
			std::cout << attr.first << ":" << attr.second << std::endl;
		}
	}

	string get(string key)
	{
		return _attributes[key];
	}

	HttpMethod method()
	{
		return _method;
	}

private:
	void getAttributes()
	{
		char* next = std::find(_begin, _end, ':');
		for (; next != _end; next = std::find(_begin, _end, ':'))
		{
			string key(_begin, next);
			_begin = next + 2;
			next = std::search(_begin, _end, CRLF, CRLF + 2);
			string val(_begin, next);
			_attributes[key] = val;
			_begin = next + 2;
		}
	}

	void getVersion()
	{
		char* next = std::search(_begin, _end, CRLF, CRLF + 2);
		_attributes["Version"] = string(_begin, next);
		_begin = next + 2;
	}

	void getUrl()
	{
		char* next = std::find(_begin, _end, ' ');
		_attributes["Url"] = string(_begin, next);
		_begin = next + 1;
	}

	void getMethod()
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
		else
		{
			_method = HttpMethod::ERROR;
		}
		_begin = empty + 1;
	}


	char* _begin;
	char* _end;
	map<string, string> _attributes;
	HttpMethod _method;
};