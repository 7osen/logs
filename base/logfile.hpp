#pragma once
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <cstring>

using std::ostream;
using std::string;
using std::fstream;
using std::ios;

class Logfile
{
public:
	Logfile(string filename,std::_Ios_Openmode flag = ios::app)
	{
		_file.open(filename, ios::in | ios::out | ios::binary | flag);
	}
	~Logfile(){}

	void Writeline();
	void Flush() { _file.flush(); }
	void SetReadPos(long pos) { _file.seekg(pos); }
	void SetWritePos(long pos) { _file.seekp(pos); }
	void ReadWrap() { _file.get(); }
	void Close() { Flush(); _file.close(); }
	bool Eof() { return _file.peek() == EOF; }
	long ReadPos() { return _file.tellg(); }
	long WritePos() { return _file.tellp(); }

	template<typename T, typename ...Args>
	void Write(T,Args ...);

	template<typename T, typename ...Args>
	void Read(T&,Args& ...);

	template<typename T>
	void Write(T);

	template<typename T>
	void Read(T&);

	template<typename T>
	ostream& operator << (T t){return _file << t;}
private:
	fstream _file;
};

template<>
void Logfile::Write(int num)
{
	_file.write((char*)&num, sizeof(int));
}

template<>
void Logfile::Write(size_t num)
{
	_file.write((char*)&num, sizeof(size_t));
	
}

template<>
void Logfile::Write(string st)
{
	Write(st.length());
	_file << st;
}

template<>
void Logfile::Write(char* ch)
{
	Write(strlen(ch));
	_file << ch;
}

template<>
void Logfile::Write(const char* ch)
{
	Write(strlen(ch));
	_file << ch;
}


template<typename T>
void Logfile::Write(T value)
{
	std::cout << "Invalid para : " << value << std::endl;
}

template<typename T,typename ...Args>
void Logfile::Write(T first,Args ... args)
{
	Write(first);
	Write(args...);

}

void Logfile::Writeline()
{
	_file << "\n";
}

template<>
void Logfile::Read(int& num)
{
	_file.read((char*)&num, sizeof(int));
}

template<>
void Logfile::Read(size_t& num)
{
	_file.read((char*)&num, sizeof(size_t));
}

template<>
void Logfile::Read(string& st)
{
	size_t len = 0;
	Read(len);
	char* ch = new char[len];
	_file.read(ch, len);
	st.assign(ch, len);
	delete[] ch;
}

template<>
void Logfile::Read(char*& ch)
{
	size_t len = 0;
	Read(len);
	_file.read(ch, len);
}

template<typename T, typename ...Args>
void Logfile::Read(T& first, Args&... args)
{

	Read(first);
	Read(args...);
}

template<typename T>
void Logfile::Read(T& value)
{
	std::cout << "Invalid para: " << value << std::endl;
}
