#include<iostream>
#include<muduo/net/EventLoop.h>
#include<muduo/net/TcpServer.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/Channel.h>
#include<muduo/net/TcpConnection.h>
#include<muduo/base/Timestamp.h>
#include<muduo/base/Logging.h>
using namespace muduo::net;
using namespace muduo;


void timer_func(const TcpConnectionPtr& connection,Timestamp start_time)
{
	double interval = timeDifference(Timestamp::now(), start_time);
	LOG_INFO << "timer func call after " << interval;
	connection->send("timer func call finish");
}

void new_message(const TcpConnectionPtr& connection, Buffer*message, Timestamp receive_time)
{
	std::cout<<"new_message"<<std::endl;
	LOG_INFO << "receive cout " << message->readableBytes() << "time is " << receive_time.toString();
	std::string str = message->retrieveAllAsString();
	LOG_INFO << "from client " << connection->name() << ":" << str;//这里其实要制定一种通信格式,不然客户端发送的多个数据无法区分 ,可以查看例子chat
	if (str == "shutdown")
	{
		connection->shutdown();
	}
	else if (str == "timer")
	{
		connection->getLoop()->runAfter(5, std::bind(timer_func,connection, Timestamp::now()));
	}
}

void new_connection(TcpConnectionPtr connection)
{
	std::cout<<"new_connection"<<std::endl;
	if (connection->connected())
	{
		//新建连接，会调用到这里
		LOG_INFO << "connected:" << connection->name();
		//作为服务端，这边通常需要拿一个map记录下所有的连接
		connection->send("hello client");
	}
	if(connection->disconnected())
	{
		//断开会调用到这里
		LOG_INFO << "disconnected" << connection->name();
		//应该需要从map中删除
	}
}

int main(int argc, char*argv[])
{
	EventLoop loop;
	uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
	InetAddress address(port);
	TcpServer server(&loop, address, "example server");

	server.setConnectionCallback(new_connection);//连接事件包括断开和新建
	server.setMessageCallback(new_message);//有消息
	server.start();
	//server.setWriteCompleteCallback()				//监听数据是否发送空了
	std::cout << "start loop" << std::endl;
	loop.loop();
	std::cout << "OK" << std::endl;
	return 0;
}