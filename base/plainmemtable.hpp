#pragma once
#include "memtable.hpp"
class plainmemtable:public memtable
{
public:
	plainmemtable(const string& name)
		:memtable(name)
	{}
	~plainmemtable()
	{}
private:
	void writeData()
	{
		iofile datafile(_dataFilename, ios::trunc);
		for (auto it = _sortlist.begin(); it != _sortlist.end(); it = it->next())
		{
			it->value = _offset;
			_offset += it->key.length();
			datafile.Write(it->key._timestamp, it->key._topic, it->key._context);
		}
		datafile.close();
	}

	void writeIndex()
	{
		string filename = _indexFilename + "w";
		iofile indexfile(filename, ios::trunc);
		indexfile.Write(_num);
		for (auto it = _sortlist.begin(); it != _sortlist.end(); it = it->next())
		{
			indexfile.Write(it->value);
		}
		indexfile.Write(_offset);
		indexfile.close();
		rename(filename.c_str(), _indexFilename.c_str());
	}
};