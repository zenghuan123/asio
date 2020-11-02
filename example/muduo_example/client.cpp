#include<iostream>
#include<muduo/net/EventLoop.h>
#include<muduo/net/TcpServer.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/Channel.h>
#include<muduo/net/TcpConnection.h>
#include<muduo/base/Timestamp.h>
#include<muduo/base/Logging.h>
#include<muduo/net/EventLoopThread.h>
#include<muduo/net/TcpClient.h>

using namespace muduo;
using namespace muduo::net;


void new_message(const TcpConnectionPtr& connection, Buffer*message, Timestamp receive_time)
{
	LOG_INFO << "receive cout " << message->readableBytes() << "time is " << receive_time.toString();
	std::string str = message->retrieveAllAsString();
	LOG_INFO << "from server " << connection->name() << ":" << str;
}
bool disconnected=false;

void connection_callback(TcpConnectionPtr connection)
{
	if (connection->connected())
	{
		LOG_INFO << "连接成功";
		connection->send("hello server");
	}
	if (connection->disconnected())
	{
		LOG_INFO << "断开连接";
		disconnected=true;
	}

}

int main(int argc, char*argv[])
{
	if (argc < 2)
	{
		std::cout << "input ip and port";
		return 1;
	}
	EventLoopThread thread;
	EventLoop *loop = thread.startLoop();
	uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
	InetAddress address(argv[1], port);
	TcpClient client(loop, address, "example client");
	client.setConnectionCallback(connection_callback);
	client.setMessageCallback(new_message);
	client.connect();
	std::string line;
	client.enableRetry();
	while (std::getline(std::cin,line))
	{
		if(! client.connection())
		{
			LOG_INFO<<"client disconnect";
			continue;
		}
		client.connection()->send(line);
		
	}
	client.disconnect();
	return 0;
}