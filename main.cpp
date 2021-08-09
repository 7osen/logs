// LogServer.cpp: 定义应用程序的入口点。
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

string getst(long long t)
{
	string st = "";
	for (int i = 1; i <= 4; i++)
	{
		t = (t + 1) << 6;
		st += (t % 26 + 'a');
		t =  (t-1) >> 5;
		st += (t % 26 + 'a');
	}
	return st;
}

void storager_test()
{
	database* s = new plainDatabase();
	s->start();
	TimeCount t;
	t.Update();
	int num = 0;
	for (;;)
	{
		Timestamp t1(getTimenow());
		message m(t1,"topic" + to_string(t1.microseconds%1000), getst(t1.hour_min_sec*t1.microseconds));
		for (int i = 1; i <= 1; i++)
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

void gdata()
{
	thread t(storager_test);
	t.detach();
	int a;
	scanf("%d", &a);
}

void server_test()
{
	LogServer ls(8080, 4);
	thread t(cmd, &ls);
	ls.start();
}

int main()
{

	//server_test();
	gdata();

}	
