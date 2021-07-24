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


int main()
{
	/*string filename = "log0";
	Kv k(filename);
	TimeCount t;
	t.Update();1
	for (int i = 1; i <= 1000000; i++)
	{
		k.Set("2323", "1", "2", "value");
	}
	cout << t.getSecond() << endl;
	t.Update();
	cout << k.Get("1", "9999999") << endl;
	cout << t.getSecond() << endl;*/
	LogServer s(8000);
	thread t(&LogServer::start,&s);
	t.detach();
	char ch;
	scanf("%c", &ch);
	s.find("2012", "2013");
	scanf("%c", &ch);
	//s.close();
}	
