﻿// LogServer.cpp: 定义应用程序的入口点。
//

#include "net/LogServer.hpp"
#include "base/timecount.hpp"
#include "base/matcher.hpp"
#include "base/lruCache.hpp"

#include <bits/stdc++.h>
using namespace std;

void print(char* begin, char* end)
{
	
	cout << string(begin, end) << endl;
}

void cmd(LogServer* l)
{
	int a;
	scanf("%d", &a);
	l->close();
}

string getstr(int n)
{
	string st = "";
	for (; n;)
	{
		st += (n%26) + 'a';
		n /= 26;
		n *= 5;
	}
	return st;
}

void storager_test()
{
	database* s = new blockDatabase();
	s->start();
	TimeCount t;
	t.Update();
	int num = 0;
	for (;;)
	{
		Timestamp t1(getTimenow());
		message m(t1,"topic" + to_string(t1.microseconds%10), getstr(t1.microseconds));
		for (int i = 1; i <= 10000; i++)
		{
		//	message m(getTimenow(), "t", "dasf");
		s->set(m);
		num++;
		if (t.getSecond() > 1.0)
			{
				t.Update();
				printf("%d\n", num);
				num = 0;
			}
		}
	}
}


int main()
{
	//database* s = new blockDatabase();
	//s->start();
	//LogServer ls(8080,4);
	thread t(storager_test);
	//thread t(cmd, &ls);
	t.detach();
	//ls.start();
	int a;
	scanf("%d", &a);
}	
