// LogServer.cpp: 定义应用程序的入口点。
//

#include "base/buffer.hpp"
#include "LogServer.hpp"
#include "base/timecount.hpp"
#include "base/kv.hpp"
#include "net/CellServer.hpp"
#include "net/TcpServer.hpp"
#include <sstream>
#include <iostream>
#include <fstream> 
#include <thread>
using namespace std;

void Onmessage(Channel* conn, Buffer* buf)
{
	cout << buf->getData() << endl;
}

void run()
{

	TcpServer s(9999);
	s.setMessageCallBack(std::bind(Onmessage, std::placeholders::_1, std::placeholders::_2));
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
