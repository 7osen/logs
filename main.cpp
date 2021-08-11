// LogServer.cpp: 定义应用程序的入口点。
//

#include "net/LogServer.hpp"

int main()
{
	LogServer ls(8080, 4);
	ls.start();
}	
