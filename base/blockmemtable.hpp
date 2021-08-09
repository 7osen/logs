#pragma once
#include "memtable.hpp"
#include <memory>

using std::shared_ptr;

struct Block
{
	string topic;
	Timestamp timestamp;
	int offset;
	Block(const string& top,const Timestamp& time, int offs) 
		:topic(top),timestamp(time),offset(offs)
	{
	}

	Block(){}
	~Block(){}
};
class blockmemtable:public memtable
{
public:
	blockmemtable(const string& name)
		:memtable(name),_blocks(std::make_shared<vector<Block> >())
	{}
	~blockmemtable(){}
	shared_ptr<vector<Block>> getBlocks(){return _blocks;}

private:
	void writeData()
	{
		iofile datafile(_dataFilename, ios::trunc);
		int i = 0;
		_blocks->resize(_sortlist.size() / BlockSize + 2);
		auto block = &(_blocks->at(0));
		int num = 0;
		block->offset = 0;
		for (auto it = _sortlist.begin(); it != _sortlist.end();)
		{
			++i;
			_offset += it->key.length();
			datafile.Write(it->key._timestamp);
			datafile.Write(it->key._topic);
			datafile.Write(it->key._context);
			auto next = it->_forward[0];
			if (i == BlockSize || next == _sortlist.end())
			{
				
				block->topic = it->key._topic;
				block->timestamp = it->key._timestamp;
				++num;
				block = &(_blocks->at(num));
				i = 0;
				block->offset = _offset;
			}
			it = next;
		}
		datafile.close();
	}

	void writeIndex()
	{
		string filename = _indexFilename + "w";
		iofile indexfile(filename, ios::trunc);
		indexfile.Write(_blocks->size());
		for (auto it = _blocks->begin(); it != _blocks->end(); it++)
		{
			indexfile.Write(it->topic);
			indexfile.Write(it->timestamp);
			indexfile.Write(it->offset);
		}
		indexfile.close();
		rename(filename.c_str(), _indexFilename.c_str());
	}

	shared_ptr<vector<Block>> _blocks;
};
