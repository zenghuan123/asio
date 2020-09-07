#include<iostream>
#include<boost/array.hpp>
#include<muduo/base/Logging.h>
#include<muduo/base/Mutex.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/TcpServer.h>
#include<boost/bind.hpp>
#include<ctime>
using namespace muduo;
using namespace muduo::net;
class TimeServer
{
public:
	TimeServer(EventLoop*loop, const InetAddress& listenAddr):server_(loop,listenAddr,"TimeServer")
	{
		server_.setConnectionCallback(std::bind(&TimeServer::onConnection, this, std::placeholders::_1));
		server_.setMessageCallback(std::bind(&TimeServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
	void start()
	{
		server_.start();
	}
private:
	void onConnection(const TcpConnectionPtr & conn)
	{
		LOG_INFO << "TimeServer on connection ";
		if (conn->connected())
		{
			time_t now = time(0);
			int32_t be32 = sockets::hostToNetwork32(static_cast<int32_t>(now));
			conn->send(&be32, sizeof(be32));
			//conn->shutdown();

		}
	}
	void onMessage(const TcpConnectionPtr&conn, Buffer*buff, Timestamp time)
	{
		string msg(buff->retrieveAllAsString());
		LOG_INFO << msg <<"  "<< conn->name();
		time_t now = ::time(0);
		int32_t be32 = sockets::hostToNetwork32(static_cast<int32_t>(now));
		conn->send(&be32, sizeof(be32));
	}
	TcpServer server_;
};

int main(int argc,char*argv[])
{	
	LOG_INFO << "server start";
	EventLoop loop;
	InetAddress listenAddr(2037);
	TimeServer server(&loop, listenAddr);
	server.start();
	loop.loop();
	return 0;
}