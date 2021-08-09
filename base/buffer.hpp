#pragma once
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

const size_t BUFFER_SIZE = 409600;

class Buffer
{
public:
	Buffer(size_t bufferSize = BUFFER_SIZE)
		:_data(new char[bufferSize]), _bufferSize(bufferSize), _Readpos(0), _Writepos(0){}

	~Buffer(){ delete _data;}
	void eat(int len) { _Readpos += len; }
	void reset();
	char* end(){return _data + _Writepos;}
	char* begin(){return _data + _Readpos;}
	void write(char* ch, int n)
	{
		memcpy(_data + _Writepos, ch, n);
		_Writepos += n;
	}
	ssize_t readFD(int fd);
	size_t availRead(){return _Writepos - _Readpos;}
	size_t availWrite(){return _bufferSize - _Writepos;}

private:
	size_t _bufferSize;
	size_t _Readpos;
	size_t _Writepos;
	char* _data;
};

ssize_t Buffer::readFD(int fd)
{
	ssize_t n = ::recv(fd, _data + _Writepos, availWrite(), MSG_DONTWAIT);
	_Writepos += n;
	return n;
}

void Buffer::reset()
{
	if (!availRead()) return;
	memcpy(_data, _data + _Readpos, availRead());
	_Writepos = availRead();
	_Readpos = 0;
}