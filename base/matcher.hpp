#pragma once
#include <string>
#include <queue>
#include <utility>
#include <vector>
#include <iostream>
#include <mutex>
#include "noncopyable.hpp"
#include "message.hpp"

using std::string;
using std::priority_queue;
using std::pair;
using std::mutex;
using std::vector;
using std::queue;
using std::stringstream;


const int MatchTimes = 1 << 16;

const int MaxFindNum = 1 << 20;


struct cmp
{
    bool operator()(const message& lhs, const message& rhs) { return rhs > lhs; }
};

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
	matcher(string key = "",int size = MatchTimes)
        :_size(size)
	{
       // if (key != "") _size = std::min(size,MatchTimes);
        autoAcNode newNode;
        _autoAc.emplace_back(newNode);
        for (auto begin = key.begin(); begin < key.end(); )
        {
            auto next = std::find(begin, key.end(), '%');
            build(string(begin, next));
            next++;
            begin = next + 2;
        }
        fail();
	}
	~matcher()
    {
        for (auto it = _s.begin();it!=_s.end();it = it->_forward[0])
        *_ss << "[" << it->key._timestamp << "] [" << it->key._topic << "]: " << it->key._context << "\n";
    }
    int size() { return _size;}
    void setStringstream(stringstream* ss){_ss = ss;}

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
    int match(const message& m) {
        int p = 0, repeat = 0;
        if (_size <= 0) return 0;
        if (_autoAc.size() > 1)
            for (int i = 0; i < m._context.length(); i++)
            {
                p = _autoAc[p].next[(int)(m._context[i])];
                for (int temp = p; temp; temp = _autoAc[temp].fail)
                    if (_autoAc[temp].sum > 0)
                    {
                        repeat++;
                        break;
                    }
            }
        else 
            repeat = 1;
        if (repeat > 0) 
        {
            std::lock_guard<mutex> _lock(_mutex);
            _size--;
            _s.push_back(m, 0);
            //*_ss << "[" << m._timestamp << "] [" <<m._topic << "]: " << m._context << "\n";
            return 1;
        }
        return 0;
    }
 
private:
    int _cnt;
    int _size;
    stringstream* _ss;
    mutex _mutex;
   // Timestamp _lasttime;
    priority_queue<message, vector<message>, > _heap;
    vector<autoAcNode> _autoAc;
};