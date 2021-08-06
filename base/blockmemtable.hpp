#pragma once
#include "memtable.hpp"
#include <memory>

using std::shared_ptr;

struct Block
{
	string* topic;
	Timestamp* timestamp;
	int offset;
};
class blockmemtable:public memtable
{
public:
	blockmemtable(const string& name)
		:memtable(name),_blocks(std::make_shared<vector<Block> >())
	{
	}
	~blockmemtable()
	{}

	shared_ptr<vector<Block>> getBlocks()
	{
		return _blocks;
	}

private:
	void writeData()
	{
		iofile datafile(_dataFilename, ios::trunc);
		int i = 0;
		Block block;
		block.offset = 0;
		for (auto it = _sortlist.begin(); it != _sortlist.end(); it = it->next())
		{
			++i;
			block.topic = &(it->key._topic);
			block.timestamp = &(it->key._timestamp);
			_offset += it->key.length();
			datafile.Write(it->key._timestamp, it->key._topic, it->key._context);
			if (i == BlockSize)
			{
				_blocks->push_back(block);
				i = 0;
				block.offset = _offset;
			}
		}
		if (i != 0) _blocks->push_back(block);
		datafile.close();
	}

	void writeIndex()
	{
		string filename = _indexFilename + "w";
		iofile indexfile(filename, ios::trunc);
		for (auto it = _blocks->begin(); it != _blocks->end(); it++)
		{
			indexfile.Write(*(it->topic),*(it->timestamp),it->offset);
		}
		indexfile.close();
		rename(filename.c_str(), _indexFilename.c_str());
	}

	shared_ptr<vector<Block>> _blocks;
};
