#pragma once

#include "TcpServer.hpp"
#include "HttpHeader.hpp"

using std::stringstream;
using std::shared_ptr;

const string NotContent = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAllow: GET,HEAD,POST\r\nAccess-Control-Allow-Headers: Content-Type,Content-Length\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: 13\r\n\r\nGET,HEAD,POST";
const string response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: ";

class HttpServer:noncopyable
{
	typedef std::function<void(char*, char*)> CallBack;
	typedef std::function<void(Connect*, shared_ptr<httpHeader>)> GetCallBack;
public:
	HttpServer(int port = 8000,int threadnum = 1)
		:_server(port)
	{
		_server.setThreadnum(threadnum);
		_server.setMessageCallBack(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
	}
	~HttpServer(){}
	void close(){_server.close();}
	void start(){_server.start();}
	void setPostCallBack(const CallBack& cb){_PostCallBack = cb;}
	void setGetCallBack(const GetCallBack& cb){_GetCallBack = cb;}


private:

	void onMessage(Connect* conn, Buffer* buf);

	TcpServer _server;
	stringstream ss;
	CallBack _PostCallBack;
	GetCallBack _GetCallBack;
};

void HttpServer::onMessage(Connect* conn, Buffer* buf)
{
	char* begin = buf->begin();
	char* end = get_http_header_end(buf->begin(), buf->end());
	for (; end != buf->end(); end = get_http_header_end(buf->begin(), buf->end()))
	{
		shared_ptr<httpHeader> header(new httpHeader(begin, end));
		end = end + 4;
		int dataLength = header->datalength;
		if (buf->end() - end >= dataLength)
		{
			buf->eat(end - begin);
			if (header->method() == HttpMethod::POST)
			{
				if (_PostCallBack)
					_PostCallBack(buf->begin(), buf->begin() + dataLength);
				conn->Write_Dontwait(NotContent.c_str(), NotContent.length());
			}
			else if (header->method() == HttpMethod::GET)
			{
				if (_GetCallBack)
					_GetCallBack(conn, header);
			}
			else if (header->method() == HttpMethod::OPTIONS)
			{
				conn->Write_Dontwait(NotContent.c_str(), NotContent.length());
			}
			buf->eat(dataLength);
		}
		else break;
		begin = buf->begin();
	}
	buf->reset();
}
