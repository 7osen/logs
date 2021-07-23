#pragma once
#include "memkv.hpp"
#include <vector>
using std::vector;
class Kv
{
public:
	Kv(string filename = "log0")
		:_filenum(1),_filename(Filepath + filename), _memkv(std::make_shared<Memkv>(_filename))
	{
		_logFileNames.emplace_back(_filename + ".log0");
		_indexFileNames.emplace_back(_filename + ".index0");
	}
	~Kv(){}
	void Restart();
	void Set(string, string, string, string);
	int Get(string, string);
	int Get(string, string, string);
	
private:
	void Roll();
	void getFileNum();
	bool file_exists(const std::string& name);
	int GetFromFile(Logfile*, string, string, string, string);
	int find(Logfile*, Logfile*, string, int);

	int _filenum;
	string _filename;
	shared_ptr<Memkv> _memkv;
	vector<string> _logFileNames;
	vector<string> _indexFileNames;
};

bool Kv::file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

void Kv::Restart()
{
	getFileNum();
	if (file_exists(_filename + ".index0"))
	{
		Roll();
		_memkv = std::make_shared<Memkv>(_filename);
	}
	else _memkv->Restart();
}

void Kv::Set(string timestamp, string userid, string topic, string context)
{
	Key k(timestamp, userid, topic);
	_memkv->Set(k, context);
	if (_memkv->Size() > MaxFileSize)
	{
		_memkv->Close();
		Roll();
		_memkv = std::make_shared<Memkv>(_filename);
	}
}

void Kv::Roll()
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

int Kv::find(Logfile* log, Logfile* index, string v,int num)
{
	string val;
	int l = 1, r = num, mid, offset;
	index->SetReadPos(sizeof(int) * num);
	index->Read(offset);
	log->SetReadPos(offset);
	log->Read(val);
	if (v > val) return r * sizeof(int) + 1;
	for (; l < r;)
	{
		mid = (l + r) >> 1;
		index->SetReadPos(sizeof(int) * mid);
		index->Read(offset);
		log->SetReadPos(offset);
		log->Read(val);
		if (val > v)
		{
			r = mid;
		}
		else l = mid + 1;
	}
	return r * sizeof(int) ;
}


int Kv::GetFromFile(Logfile* result, string logFilename, string indexFilename, string start_time, string end_time)
{
	Logfile log(logFilename);
	Logfile index(indexFilename);
	int num = 0,offset = 0,ret = 0;
	index.Read(num);
	int begin = find(&log, &index, start_time, num);
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
		*result << timestamp << "-" << userid << "-" << topic << ": " << context << "\n";
	}
	return ret;
}


int Kv::Get(string start_time, string end_time)
{
	Logfile result(Filepath + "result.log", ios::trunc);
	int ret = _memkv->Get(&result, start_time, end_time);
	for (int i = 1; i < _filenum; i++)
	{
		ret += GetFromFile(&result,_logFileNames[i],_indexFileNames[i],start_time,end_time);
	}
	result.Close();
	return ret;
}

int Kv::Get(string userid, string start_time, string end_time)
{
	
}

void Kv::getFileNum()
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