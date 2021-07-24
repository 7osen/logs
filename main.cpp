// LogServer.cpp: 定义应用程序的入口点。
//

#include "net/LogServer.hpp"
#include "base/timecount.hpp"
#include <bits/stdc++.h>
#include <atomic>
using namespace std;

void print(char* begin, char* end)
{
	
	cout << string(begin, end) << endl;
}

void run()
{
	LogServer s(8001);
	s.start();
}	

int main()
{
	/*string filename = "log0";
	Kv k(filename);
	TimeCount t;
	t.Update();
	for (int i = 1; i <= 1000000; i++)
	{
		k.Set("2323", "1", "2", "value");
	}
	cout << t.getSecond() << endl;
	t.Update();
	cout << k.Get("1", "9999999") << endl;
	cout << t.getSecond() << endl;*/
	thread t(run);
	t.detach();
	char ch;
	scanf("%c", &ch);
	
}	
