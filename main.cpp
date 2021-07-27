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

	//LogServer s(8000,1);
	//thread t(&LogServer::start,&s);
	//t.detach();

	//s.close();
	//s.find("2012", "2013");
	TimeCount t;
	storager s;
	t.Update();
	for (int i = 0; i < 1000000; i++)
	{
		message m("123", "", "789", "123");
		s.Set(m);
		
	}
	//s.Flush();
	cout << t.getSecond() << endl;
	cout << num << endl;
	s.Get("000", "456");
	char ch;
	scanf("%c", &ch);
}	
