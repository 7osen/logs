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


void storager_test()
{
	database* s = new blockDatabase();
	s->start();
	TimeCount t;
	t.Update();
	int num = 0;
	for (;;)
	{
		for (int i = 1; i <= 10000; i++)
		{
		//	message m(getTimenow(), "t", "dasf");
	Timestamp t1(getTimenow());
	message m(t1,"topic" + to_string(t1.microseconds%10), to_string(t1.microseconds));
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
	LogServer ls(8000,2);
	//thread t(storager_test);
	thread t(cmd, &ls);
	t.detach();
	ls.start();
	//int a;
	//scanf("%d", &a);
	//database* d = new blockDatabase();
	//d->start();
	//shared_ptr<httpHeader> h = std::make_shared<httpHeader>();
	//h->topic = "topic5";
	//h->begin = "20210807-15:20";//statei.range(0);
	//h->end = "20210807-15:30";
	//h->num = 1;
	//stringstream ss;
	//d->get(&ss, h);
	//cout << ss.str() << endl;
	
}	
