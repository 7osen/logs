#pragma once
#include "storager.hpp"
#include "plainmemtable.hpp"
class plaintableStorager:public storager
{
public:
	plaintableStorager(const string& name = "log")
		:storager(name)
	{
		_mems = createMemtable();
		restart();
		_tempnum++;
		_tempfile = new iofile(_filename + ".temp");
		startflush();
	}
	~plaintableStorager()
	{
		delete _mems;
	}

private:
	memtable* createMemtable()
	{
		return new plainmemtable(getTimenow());
	}

	int getFromFile(std::stringstream* ss, const string& logFilename, const string& indexFilename, const message& start_time, const message& end_time, int num)
	{
		iofile log(logFilename);
		iofile index(indexFilename);
		int n = 0, offset = 0, ret = 0;
		index.Read(n);
		int begin = find(&log, &index, start_time, n);
		int end = find(&log, &index, end_time, n);
		index.setReadPos(begin);
		index.Read(begin);
		index.setReadPos(end);
		index.Read(end);
		string timestamp;
		string topic;
		string context;
		log.setReadPos(begin);
		for (; log.readPos() < end && !log.eof();)
		{
			if (ret == num) return ret;
			ret++;
			log.Read(timestamp, topic, context);
			*ss << "[" << timestamp << "] [" << topic << "]: " << context << "\n";
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