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
	matcher(string key = "")
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

    void setStringstream(stringstream* ss)
    {
        _ss = ss;
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
        if (_autoAc[0].sum == 0)
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
            *_ss << "[" << m._timestamp << "] [" <<m._topic << "]: " << m._context << "\n";
            return 1;
        }
        return 0;
    }
 

private:
    int _cnt;
    stringstream* _ss;
    vector<autoAcNode> _autoAc;
    vector<message> _messages;
};