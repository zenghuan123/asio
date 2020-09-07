#include "muduo/base/Logging.h"
#include "muduo/net/Endian.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpClient.h"
#include <iostream>

#include <utility>

#include <stdio.h>


using namespace muduo;
using namespace muduo::net;

class TimeClient : noncopyable
{
public:
	TimeClient(EventLoop* loop, const InetAddress& serverAddr)
		: loop_(loop),
		client_(loop, serverAddr, "TimeClient")
	{
		client_.setConnectionCallback(
			std::bind(&TimeClient::onConnection, this, _1));
		client_.setMessageCallback(
			std::bind(&TimeClient::onMessage, this, _1, _2, _3));
		// client_.enableRetry();
	}

	void connect()
	{
		client_.connect();
	}

private:

	EventLoop* loop_;
	TcpClient client_;

	void onConnection(const TcpConnectionPtr& conn)
	{

		if (!conn->connected())
		{
			//loop_->quit();
			std::cout << "not connect" << std::endl;
		}
		
		loop_->runEvery( 1.0, std::bind(&TimeClient::every_time,this, conn));
	}
	void every_time(const TcpConnectionPtr& conn)
	{
		conn->send("12345", 5);
		std::cout << "every_time" << std::endl;
	}

	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
	{
		if (buf->readableBytes() >= sizeof(int32_t))
		{
			const void* data = buf->peek();
			int32_t be32 = *static_cast<const int32_t*>(data);
			buf->retrieve(sizeof(int32_t));
			time_t time = sockets::networkToHost32(be32);
			Timestamp ts(implicit_cast<uint64_t>(time) * Timestamp::kMicroSecondsPerSecond);
			LOG_INFO << "Server time = " << time << ", " << ts.toFormattedString();
			
		}
		else
		{
			LOG_INFO << conn->name() << " no enough data " << buf->readableBytes()
				<< " at " << receiveTime.toFormattedString();
		}
	}
};

int main(int argc, char* argv[])
{
	LOG_INFO << "client start ";
	if (argc > 1)
	{
		EventLoop loop;
		InetAddress serverAddr(argv[1], 2037);

		TimeClient timeClient(&loop, serverAddr);
		timeClient.connect();
		loop.loop();
	}
	else
	{
		EventLoop loop;
		InetAddress serverAddr("127.0.0.1", 2037);

		TimeClient timeClient(&loop, serverAddr);
		timeClient.connect();
		loop.loop();
		std::cout << "end" << std::endl;

		
	}
}

