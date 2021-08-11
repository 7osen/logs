#pragma once
#include <unordered_map>
#include <list>
#include <utility>
#include <mutex>

using std::mutex;
using std::unordered_map;
using std::list;
using std::pair;
using std::make_pair;


const int LruCacheSize = 24;


template<typename Key,typename Value,typename Iter>
class LRUCache
{
	typedef list<pair<Key, Value>> List;
public:
	LRUCache(int size = LruCacheSize)
		:_size(size)
	{}

	Value get(const Key& k)
	{
		if (_map.find(k) == _map.end())
			return Value();
		else
		{
			push(k, ((*_map[k]).second));
			return ((*_map[k]).second);
		}
	}

	void push(const Key& k,const Value& v)
	{
		std::lock_guard<mutex> _lock(_mutex);
		if (_map.find(k) != _map.end())
		{
			_list.push_front(make_pair<Key, Value>((Key)k, (Value)v));
			_list.erase(_map[k]);
			_map[k] = _list.begin();
			return;
		}
		if (_list.size() >= _size)
		{
			_map.erase(_map.find(_list.back().first));
			_list.pop_back();
		}
		if (_map.find(k) == _map.end())
		{
			_list.push_front(make_pair<Key, Value>((Key)k, (Value)v));
			_map[k] = _list.begin();
			return;
		}
	}

	~LRUCache()
	{}

private:
	int _size;
	mutex _mutex;
	List _list;
	unordered_map<Key,Iter> _map;
};
