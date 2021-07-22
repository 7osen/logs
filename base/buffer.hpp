#pragma once
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

const size_t BUFFER_SIZE = 40960;
class Buffer
{
public:
	Buffer(size_t bufferSize = BUFFER_SIZE)
		:_data(new char[bufferSize]), _bufferSize(bufferSize), _Readpos(0), _Writepos(0){}

	~Buffer()
	{
		delete[] _data;
	}

	char* getData()
	{
		return _data;
	}

	ssize_t readFD(int fd)
	{
		ssize_t n = ::recv(fd, _data + _Writepos, availWrite(),MSG_DONTWAIT);
		_Writepos += n;
		return n;
	}

	void reset()
	{
		memcpy(_data, _data + _Readpos, availRead());
		_Writepos = availRead();
		_Readpos = 0;
	}

	size_t availRead()
	{
		return _Writepos - _Readpos;
	}

	size_t availWrite()
	{
		return _bufferSize - _Writepos;
	}

private:
	size_t _bufferSize;
	size_t _Readpos;
	size_t _Writepos;
	char* _data;
};