#pragma once

#include "TcpServer.hpp"
#include "HttpHeader.hpp"

class HttpServer
{
	typedef std::function<void(char*, char*)> CallBack;
public:
	HttpServer(int port,int threadnum = 1)
		:_server(port)
	{
		_server.setThreadnum(threadnum);
		_server.setMessageCallBack(std::bind(&HttpServer::Onmessage, this, std::placeholders::_1, std::placeholders::_2));
	}

	~HttpServer()
	{

	}

	void start()
	{
		_server.start();
	}

	void setOnMessageCallBack(const CallBack& cb)
	{
		_OnMessageCallBack = cb;
	}


private:

	void Onmessage(Channel* conn, Buffer* buf)
	{
		char* begin = buf->begin();
		char* end = get_http_header_end(buf->begin(), buf->end());
		for (; end != buf->end(); end = get_http_header_end(buf->begin(), buf->end()))
		{
			httpHeader header(begin, end);
			std::cout << header.method() << std::endl;
			end = end + 4;
			int dataLength = std::stoi(header.get("Content-Length"));
			if (buf->end() - end >= dataLength)
			{
				buf->getData(end - begin);
				if (_OnMessageCallBack)
				{
					_OnMessageCallBack(buf->begin(), buf->begin() + dataLength);
				}
				buf->getData(dataLength);
				conn->Write(header.get("Version") + " " + "200 OK\r\nContent-Length: 0\r\n\r\n");
			}
			else break;
		}
		buf->reset();
	}
	TcpServer _server;
	CallBack _OnMessageCallBack;
};
