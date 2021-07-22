// LogServer.cpp: 定义应用程序的入口点。
//

#include "LogServer.hpp"
#include "base/timecount.hpp"
#include "base/kv.hpp"
#include "net/Epoller.hpp"
#include <sstream>
#include <iostream>
#include <fstream> 
using namespace std;



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
	Epoller p;
}	
