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

string randomst(int n)
{
	string ret = "";
	for (; n--;)
	{
		ret += ('a' + random() % 26);
		srand(rand());
	}
	return ret;
}

void storager_test()
{
	storager* s = new blocktableStorager();
	s->start();
	TimeCount t;
	t.Update();
	int num = 0;
	for (;;)
	{
		{
		message m(getTimenow(),"topic" + to_string(random()%10),randomst(random()%10));
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
//	thread t(storager_test);
	thread t(cmd, &ls);
	t.detach();
	ls.start();
//	int a;
	//scanf("%d", &a);
}	
