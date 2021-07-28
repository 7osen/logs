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

	LogServer ls(8000);
	thread t(&LogServer::start,&ls);
	t.detach();

	char ch;
	scanf("%c", &ch);
	ls.close();
}	
