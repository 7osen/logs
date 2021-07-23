#pragma once
#include <thread>
#include <vector>
#include "CellServer.hpp"

#define INVALID_SOCKET			(~0)
#define SOCKET_ERROR            (-1)

using std::vector;

class TcpServer
{
	typedef function<void(Channel*, Buffer*)> MessageCallBack;
public:
	TcpServer(int port)
		:_port(port),
		_threadnum(1),
		_running(false)
	{}

	~TcpServer()
	{
		for (int i = 0; i < _threadnum; i++)
		{
			_servers[i]->close();
		}
		close(_fd);
	}

	void setMessageCallBack(const MessageCallBack& cb)
	{
		_MessageCallBack = cb;
	}

	void setThreadnum(int n)
	{
		_threadnum = n;
	}

	void start()
	{
		bind();
		startCellserver();
		listen();
		_running = true;
		accept();
	}

private:

	void accept()
	{
		while (_running)
		{
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			int _cSock = INVALID_SOCKET;
			_cSock = ::accept(_fd, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
			if (_cSock != INVALID_SOCKET)
			{
				CellServer* minServer = _servers[0];
				for (auto server : _servers)
				{
					if (server->size() < minServer->size())
					{
						minServer = server;
					}
				}
				minServer->addNewClient(_cSock);
			}
		}
	}

	void listen()
	{
		int retlis = ::listen(_fd, 5);
		if (SOCKET_ERROR == retlis)
		{
			printf("listen error!\n");
		}
		else
		{
			printf("listen success!\n");
		}
	}

	void startCellserver()
	{
		for (int i = 0; i < _threadnum; i++)
		{
			auto cserver = new CellServer();
			if (_MessageCallBack)
			{
				cserver->setMessageCallBack(_MessageCallBack);
			}
			_servers.push_back(cserver);
			cserver->start();
		}
	}

	void bind()
	{
		_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(_port);
		_sin.sin_addr.s_addr = INADDR_ANY;
		int retbind = ::bind(_fd, (sockaddr*)&_sin, sizeof(_sin));
		if (-1 == retbind)
		{
			printf("bind error!\n");
		}
		else
		{
			printf("bind success! ip = %s ... port = %d \n", inet_ntoa(_sin.sin_addr), _port);
		}
	}

	bool _running;
	int _port;
	int _fd;
	int _threadnum;
	MessageCallBack _MessageCallBack;
	vector<CellServer*> _servers;
};