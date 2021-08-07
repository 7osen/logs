#pragma once
#include "database.hpp"
#include "plainmemtable.hpp"
class plainDatabase:public database
{
public:
	plainDatabase(const string& name = "log")
		:database(name)
	{}
	~plainDatabase()
	{
		delete _mems;
	}

private:
	memtable* createMemtable()
	{
		return new plainmemtable(getTimenow());
	}
	void push_cache(memtable*)
	{}
	void restart()
	{}
	int get_from_file(matcher* match, logfile* file, const message& start_time, const message& end_time, int num)
	{
		iofile log(file->datafilename());
		iofile index(file->indexfilename());
		int n = 0, offset = 0, ret = 0;
		index.Read(n);
		int begin = find(&log, &index, start_time, n);
		int end = find(&log, &index, end_time, n);
		index.setReadPos(begin);
		index.Read(begin);
		index.setReadPos(end);
		index.Read(end);
		Timestamp timestamp;
		string topic;
		string context;
		log.setReadPos(begin);
		for (; log.readPos() < end && !log.eof();)
		{
			if (ret == num) return ret;
			log.Read(timestamp, topic, context);
			ret += match->match(message(timestamp, topic, context));
		}
		return ret;
	}

	int find(iofile* log, iofile* index, const message& v, int num)
	{
		message val;
		int l = 1, r = num, mid, offset;
		index->setReadPos(sizeof(int) * num);
		index->Read(offset);
		log->setReadPos(offset);
		log->Read(val._timestamp, val._topic);
		if (v > val) return (r + 1) * sizeof(int);
		for (; l < r;)
		{
			mid = (l + r) >> 1;
			index->setReadPos(sizeof(int) * mid);
			index->Read(offset);
			log->setReadPos(offset);
			log->Read(val._timestamp, val._topic);
			if (v > val)
			{
				l = mid + 1;
			}
			else r = mid;
		}
		return r * sizeof(int);
	}
};