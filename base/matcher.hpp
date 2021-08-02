#pragma once
#include <string>
#include <queue>
#include <utility>
#include <vector>
#include <iostream>
#include "noncopyable.hpp"
#include "message.hpp"

using std::string;
using std::priority_queue;
using std::pair;
using std::vector;
using std::queue;
using std::stringstream;

struct autoAcNode {
    int num, sum, fail, tim, fa, next[256];
    autoAcNode()
        :num(0), sum(0), fail(0), fa(0)
    {
        memset(next, 0, sizeof(next));
    }
};

class matcher
{
public:
	matcher(int num,string key = "")
        :_num(num)
	{
        autoAcNode newNode;
        _autoAc.emplace_back(newNode);
        for (auto begin = key.begin(); begin < key.end(); )
        {
            auto next = std::find(begin, key.end(), '-');
            build(string(key.begin(), next));
            begin = ++next;
        }
        fail();
	}
	~matcher()
	{

	}
    void build(string s) {
        int now = 0;
        for (int i = 0; i < s.length(); i++)
        {
            if (_autoAc[now].next[(int)s[i]]) now = _autoAc[now].next[(int)s[i]];
            else {
                autoAcNode newNode;
                newNode.num = (int)s[i];
                newNode.fa = now;
                _autoAc[now].next[(int)s[i]] = _autoAc.size();
                now = _autoAc.size();
                _autoAc.emplace_back(newNode);
            }
        }
        _autoAc[now].sum++;
    }
    void fail() {
        int h = 0, t = 0;
        queue<int > q;
            for (int i = 0; i < 256; i++) 
              if (_autoAc[0].next[i])
                {
                    q.push(_autoAc[0].next[i]);
                    _autoAc[_autoAc[0].next[i]].fail = 0;
                }
        while (!q.empty()) 
        {
            int now = q.front();
            q.pop();
            for (int i = 0; i < 256; i++)
                if (_autoAc[now].next[i]) 
                {
                    q.push(_autoAc[now].next[i]);
                    _autoAc[_autoAc[now].next[i]].fail = _autoAc[_autoAc[now].fail].next[i];
                    if (!_autoAc[_autoAc[now].next[i]].fail)
                        _autoAc[_autoAc[now].next[i]].fail = 0;
                }
                else _autoAc[now].next[i] = _autoAc[_autoAc[now].fail].next[i];
        }
    }
    void match(const message& m) {
        int p = 0, repeat = 0;
        for (int i = 0; i < m._context.length(); i++) 
        {
            p = _autoAc[p].next[(int)(m._context[i])];
            for (int temp = p; temp; temp = _autoAc[temp].fail)
                if (_autoAc[temp].sum > 0)
                {
                    repeat++;
                }
        }
        if (repeat > 0)
        {
            _maxheap.push(std::make_pair(repeat, std::move(m)));
            if (_maxheap.size() > _num)
                _maxheap.pop();
        }
    }
 
    void print(stringstream* ss)
    {
        for (; !_maxheap.empty();)
        {
            *ss << "[" << _maxheap.top().second._timestamp << "] [" << _maxheap.top().second._topic << "]: " << _maxheap.top().second._context << "\n";
            _maxheap.pop();
        }
    }

private:
	int _num;
    int _cnt;
    vector<autoAcNode> _autoAc;
	priority_queue<pair<int, message> > _maxheap;
};