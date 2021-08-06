#pragma once
#include <memory>
#include "storager.hpp"
#include "blockmemtable.hpp"
#include "lruCache.hpp"

using std::shared_ptr;
using std::list;
using std::string;
using std::vector;


class blocktableStorager:public storager
{
	typedef shared_ptr<vector<Block>> BlocksPtr;
public:
	blocktableStorager(const string& name = "log")
		:storager(name)
	{

	}
	~blocktableStorager()
	{
		delete _mems;
	}

private:

	void memchange(memtable* table)
	{
		_lru.push(table->name(),dynamic_cast<blockmemtable*>(table)->getBlocks());
	}

	void restart()
	{
		if (_metadata.size() > 0)
			for (auto it = max(_metadata.end() - LruCacheSize, _metadata.begin()); it != _metadata.end(); it++)
			{
				getBlocks(*it);
			}
	}

	int find(iofile* file, const BlocksPtr& blocks,const message& v)
	{
		int l = 0;
		int r = blocks->size() - 1;
		string st("1");
		for (; l < r;)
		{
			int mid = (l + r) >> 1;
			message m(*(blocks->at(mid).timestamp), *(blocks->at(mid).topic),st);
			if (v > m)
			{
				l = mid + 1;
			}
			else r = mid;
		}
		if (r < 0) return 0;
		file->setReadPos(blocks->at(r).offset);
		int offset = file->readPos();
		string  topic, context;
		Timestamp timestamp;
		for (; !file->eof();)
		{
			file->Read(timestamp, topic, context);
			message m(timestamp, topic, context);
			if (v > m)
			{
				offset = file->readPos();
			}
			else 
				return offset;
		}
		return offset;
	}


	BlocksPtr getBlocks(logfile* file)
	{
		BlocksPtr blocks;
		blocks = _lru.get(file->basename());
		if (!blocks)
		{
			printf("no in cache \n");
			blocks = std::make_shared<vector<Block> >();
			iofile indexfile(file->indexfilename());
			for (; !indexfile.eof();)
			{
				Block block;
				string topic;
				Timestamp timestamp;
				indexfile.Read(topic, timestamp, block.offset);
				block.topic = new string(topic);
				block.timestamp = new Timestamp(timestamp);
				blocks->push_back(std::move(block));
			}
			_lru.push(file->basename(), blocks);
		}
		return blocks;
	}

	int getFromFile(matcher* match, logfile* file, const message& begin, const message& end, int num)
	{
		BlocksPtr blocks = getBlocks(file);
		iofile* datafile = new iofile(file->datafilename());
		int beginOffset = find(datafile, blocks, begin);
		int endOffset = find(datafile, blocks, end);
		datafile->setReadPos(beginOffset);
		string  topic, context;
		Timestamp timestamp;
		int ret = 0;
		while (!datafile->eof() && datafile->readPos() < endOffset)
		{
			datafile->Read(timestamp, topic, context);
			ret += match->match(message(timestamp, topic, context));
			if (ret == num) return ret;
		}
		return ret;
	}

	memtable* createMemtable()
	{
		return new blockmemtable(getTimenow());
	}

	LRUCache<string, BlocksPtr,list<pair<string, BlocksPtr> >::iterator> _lru;
};