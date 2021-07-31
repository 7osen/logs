#pragma once
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <cstring>
#include "noncopyable.hpp"
#include "buffer.hpp"
#include "setting.hpp"

using std::ostream;
using std::string;
using std::fstream;
using std::ios;

class logfile: noncopyable
{
public:
	logfile(string filename, std::_Ios_Openmode flag = ios::app)
		:_filename(filename) , _times(0)
	{
		_file.open(filename, ios::in | ios::out | ios::binary | flag);
	}
	~logfile(){}

	void Writeline();
	void flush();
	void setReadPos(int32_t pos) { _file.seekg(pos); }
	void setWritePos(int32_t pos) { _file.seekp(pos); }
	void readWrap() { _file.get(); }
	void close() { _file.flush(); _file.close(); }
	bool eof() { return _file.peek() == EOF; }
	const string& name() { return _filename; };
	int32_t readPos() { return _file.tellg(); }
	int32_t writePos() { return _file.tellp(); }

	template<typename T, typename ...Args>
	void Write(const T&,const Args& ...);

	template<typename T, typename ...Args>
	void Read(T&,Args& ...);

	template<typename T>
	void Write(const T&);

	template<typename T>
	void Read(T&);

	template<typename T>
	ostream& operator << (T t){return _file << t;}
private:
	string _filename;
	fstream _file;
	int _times;
	Buffer _buffer;
};

void logfile::flush()
{
	_times++;
	if (_times >= EveryFlush)
	{
		_file.flush();
		_times = 0;
	}
}

template<>
void logfile::Write(const int& num)
{
	_file.write((char*)&num, sizeof(int));
}

template<>
void logfile::Write(const size_t& num)
{
	_file.write((char*)&num, sizeof(size_t));
	
}

template<>
void logfile::Write(const string& st)
{
	Write(st.length());
	_file << st;
}


template<typename T>
void logfile::Write(const T& value)
{
	std::cout << "Invalid para : " << value << std::endl;
}

template<typename T,typename ...Args>
void logfile::Write(const T& first,const Args& ... args)
{
	Write(first);
	Write(args...);

}

void logfile::Writeline()
{
	_file << "\n";
}

template<>
void logfile::Read(int& num)
{
	_file.read((char*)&num, sizeof(int));
}

template<>
void logfile::Read(size_t& num)
{
	_file.read((char*)&num, sizeof(size_t));
}

template<>
void logfile::Read(string& st)
{
	size_t len = 0;
	Read(len);
	_file.read(_buffer.begin(), len);
	st.assign(_buffer.begin(), len);
}

template<>
void logfile::Read(char*& ch)
{
	size_t len = 0;
	Read(len);
	_file.read(ch, len);
}

template<typename T, typename ...Args>
void logfile::Read(T& first, Args&... args)
{

	Read(first);
	Read(args...);
}

template<typename T>
void logfile::Read(T& value)
{
	std::cout << "Invalid para: " << value << std::endl;
}
