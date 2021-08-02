#pragma once

#include "TcpServer.hpp"
#include "HttpHeader.hpp"

using std::stringstream;

class HttpServer:noncopyable
{
	typedef std::function<void(char*, char*)> CallBack;
	typedef std::function<void(string&,const httpHeader&)> GetCallBack;
public:
	HttpServer(int port = 8000,int threadnum = 1)
		:_server(port)
	{
		_server.setThreadnum(threadnum);
		_server.setMessageCallBack(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
	}

	~HttpServer()
	{

	}

	void close()
	{
		_server.close();
	}

	void start()
	{
		_server.start();
	}

	void setPostCallBack(const CallBack& cb)
	{
		_PostCallBack = cb;
	}
	
	void setGetCallBack(const GetCallBack& cb)
	{
		_GetCallBack = cb;
	}


private:

	void onMessage(Connect* conn, Buffer* buf)
	{
		char* begin = buf->begin();
		char* end = get_http_header_end(buf->begin(), buf->end());
		for (; end != buf->end(); end = get_http_header_end(buf->begin(), buf->end()))
		{
			httpHeader header(begin, end);
			end = end + 4;

			int dataLength = header.datalength;
			if (buf->end() - end >= dataLength)
			{
				string response = "";
				buf->addReadpos(end - begin);
				if (header.method() == HttpMethod::POST)
				{
					if (_PostCallBack) 
						_PostCallBack(buf->begin(), buf->begin() + dataLength);
				}
				else if (header.method() == HttpMethod::GET)
				{
					if (_GetCallBack)
					_GetCallBack(response,header);
				}
				onRequest(conn, header.version, response);
				buf->addReadpos(dataLength);
				
			}
			else break;
			begin = buf->begin();
		}
		buf->reset();
	}

	void onRequest(Connect* conn,const string& version, const string& st)
	{
		stringstream ss;
		int l = st.length();
		if (l == 0) 
			ss << version << " " << "204 NotContent\r\n\r\n";
		else 
			ss << version << " " << "200 OK\r\nContent-Length: " << l << "\r\n\r\n" << st;
		//std::cout << ss.str() << std::endl;
		conn->Write(ss.str());
	}

	TcpServer _server;
	CallBack _PostCallBack;
	GetCallBack _GetCallBack;
};
