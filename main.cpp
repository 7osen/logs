// LogServer.cpp: 定义应用程序的入口点。
//

#include "net/LogServer.hpp"
#include "base/timecount.hpp"
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

void run()
{
	string s11 = "1";
	for (int i = 1; i < 2; i++) s11 += "1";
	message m("2012", "topi", s11);
	storager st;
	int num = 0;
	TimeCount t;
	t.Update();
	for (int i = 1; i <= 10000000; i++)
	{
		st.set(m);
		num++;
	}
	cout << t.getSecond() << endl;
	t.Update();
	stringstream ss;
	st.get(&ss, "topi", "2012", "2012", 100);
	cout << t.getSecond() << endl;
	cout << ss.str().length() << endl;
	t.Update();
	stringstream s;
	st.get(&s, "topi", "2013", "2013", 100);
	cout << t.getSecond() << endl;
	cout << s.str().length() << endl;
}

int main()
{

	LogServer ls(8000);
	//thread t(run);
	thread t(cmd, &ls);
	t.detach();
	ls.start();
}	
