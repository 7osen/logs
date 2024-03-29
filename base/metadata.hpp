#pragma once

#include <string>
#include <vector>
#include "noncopyable.hpp"
#include "setting.hpp"
#include "iofile.hpp"

using std::vector;
using std::string;

const int MaxFileNum = 64;

bool file_exists(const string& name) {
	struct stat file;
	return (stat(name.c_str(), &file) == 0);
}

class logfile:noncopyable
{
public:
	logfile(const string& name,const Timestamp& max_time,const Timestamp& min_time)
		:_basename(name),_max_time(max_time),_min_time(min_time)
	{}
	~logfile(){}
	bool inCache(){	return _inCache;	}
	const Timestamp& max_time() { return _max_time; }
	const Timestamp& min_time() { return _min_time; }
	string indexfilename() { return _basename + ".index";}
	string datafilename() { return _basename + ".data";}
	string basename() { return _basename; }


private:
	bool _inCache;
	string _basename;
	Timestamp _max_time;
	Timestamp _min_time;
};




class metadata
{
	typedef vector<logfile*>::iterator Iterator;
public:
	metadata(){}
	~metadata(){}
	int size(){return _files.size();}
	Iterator begin() { return _files.begin(); }
	Iterator end() { return _files.end(); }



	void restart()
	{
		string basename;
		Timestamp min_time;
		Timestamp max_time;
		iofile metadata(Filepath + "metafile.data");
		while (!metadata.eof())
		{
			 metadata.Read(basename, min_time, max_time);
			 if (file_exists(basename + ".index"))
				 _files.emplace_back(new logfile(basename, min_time, max_time));
		}
		metadata.close();
	}

	void push_back(logfile* file)
	{
		_files.emplace_back(file);
		if (_files.size() > MaxFileNum)
		{
			for (int i = 0; i < MaxFileNum / 5; i++)
			{
				remove((*begin())->datafilename().c_str());
				remove((*begin())->indexfilename().c_str());
				_files.erase(_files.begin());
			}
			rewrite();
		}
		else
		{
			iofile metadata(Filepath + "metafile.data");
			metadata.Write(file->basename(), file->min_time(), file->max_time());
			metadata.close();
		}

	}

private:
	void rewrite()
	{
		iofile metadata(Filepath + "metafile.dataw");
		for (auto it = begin(); it != end(); it++)
		{
			metadata.Write((*it)->basename(), (*it)->min_time(), (*it)->min_time());
		}
		metadata.close();
		rename((Filepath + "metafile.dataw").c_str(), (Filepath + "metafile.data").c_str());
	}


	vector<logfile*> _files;
};


