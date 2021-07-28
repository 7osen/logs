#pragma once
#include "memtable.hpp"
#include <vector>
using std::vector;

class storager:noncopyable
{
public:
	storager(string filename = "log")
		:_filenum(1),_filename(Filepath + filename), _mems(std::make_shared<memtable>(_filename))
	{
		_logFileNames.emplace_back(_filename + ".log0");
		_indexFileNames.emplace_back(_filename + ".index0");

	}
	~storager(){}
	void Restart();
	void Set(const message&);
	void Flush() { _mems->Flush(); }
	int Get(std::stringstream*,const string&, const string&, const string&);
	
private:
	void Roll();
	void getFileNum();
	bool file_exists(const std::string& name);
	int GetFromFile(std::stringstream*, string, string, const message&, const message&);
	int find(Logfile*, Logfile*, message, int);

	int _filenum;
	string _filename;
	shared_ptr<memtable> _mems;
	vector<string> _logFileNames;
	vector<string> _indexFileNames;
};

bool storager::file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

void storager::Restart()
{
	getFileNum();
	if (file_exists(_filename + ".index0"))
	{
		Roll();
		_mems = std::make_shared<memtable>(_filename);
	}
	else _mems->Restart();
}


void storager::Roll()
{

	if (_filenum < MaxFileNum)
	{
		_filenum++;
		_logFileNames.push_back(_filename + ".log" + std::to_string(_filenum - 1));
		_indexFileNames.push_back(_filename + ".index" + std::to_string(_filenum - 1));
	}
	else
	{
		remove(_logFileNames[_filenum - 1].c_str());
		remove(_indexFileNames[_filenum - 1].c_str());
	}
	string newfilename = _logFileNames[_filenum - 1];
	string newindexname = _indexFileNames[_filenum - 1];
	string oldfilename;
	string oldindexname;
	for (int i = _filenum - 2; i >= 0; i--)
	{
		oldfilename = _logFileNames[i];
		oldindexname = _indexFileNames[i];
		rename(oldfilename.c_str(), newfilename.c_str());
		rename(oldindexname.c_str(), newindexname.c_str());
		newfilename = oldfilename;
		newindexname = oldindexname;
	}
}

int storager::find(Logfile* log, Logfile* index, message v,int num)
{
	message val;
	int l = 1, r = num, mid, offset;
	index->SetReadPos(sizeof(int) * num);
	index->Read(offset);
	log->SetReadPos(offset);
	log->Read(val._timestamp,val._username,val._topic,val._context);
	if (v > val) return r * sizeof(int) + 1;
	for (; l < r;)
	{
		mid = (l + r) >> 1;
		index->SetReadPos(sizeof(int) * mid);
		index->Read(offset);
		log->SetReadPos(offset);
		log->Read(val._timestamp, val._username, val._topic, val._context);
		if (val > v)
		{
			r = mid;
		}
		else l = mid + 1;
	}
	return r * sizeof(int) ;
}


int storager::GetFromFile(std::stringstream* ss, string logFilename, string indexFilename, const message& start_time,const message& end_time)
{
	Logfile log(logFilename);
	Logfile index(indexFilename);
	int num = 0,offset = 0,ret = 0;
	index.Read(num);
	int begin = find(&log, &index,start_time, num);
	int end = find(&log, &index, end_time, num);
	index.SetReadPos(begin);
	string timestamp;
	string userid;
	string topic;
	string context;
	for (;index.ReadPos() < end;)
	{
		index.Read(offset);
		log.SetReadPos(offset);
		log.Read(timestamp, userid, topic, context);
		log.ReadWrap();
		ret++;
		*ss << timestamp << "-" << userid << "-" << topic << ": " << context << "\n";
	}
	return ret;
}


int storager::Get(std::stringstream* ss,const string& username,const string& start_time,const string& end_time)
{
	message start(start_time, username, "", "");
	message end(end_time, username, "", "");
	int ret = _mems->Get(ss, start, end);
	for (int i = 1; i < _filenum; i++)
	{
		ret += GetFromFile(ss, _logFileNames[i], _indexFileNames[i], start, end);
	}
	return ret;
}

void storager::getFileNum()
{
	string name;
	for (int i = 0; i < MaxFileNum; i++)
	{
		name = _filename + ".log" + std::to_string(i);
		if (file_exists(name))
		{
			_logFileNames.emplace_back(name);
			_indexFileNames.emplace_back(_filename + ".index" + std::to_string(i));
			_filenum = i + 1;
		}
		else break;
	}
}

void storager::Set(const message& m)
{
	_mems->Set(m);
	if (_mems->Size() > MaxFileSize)
	{
		_mems->Close();
		Roll();
		_mems = std::make_shared<memtable>(_filename);
	}
}
