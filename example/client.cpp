#include<iostream>
#include "example.h"
#include "asio_server.h"
#include <thread>

int main(int argc, char* argv[])
{

	//client_run();
	asio::io_context io_context;
	asio_server::TcpClient c(io_context,"127.0.0.1",999999);
	c.connect();
	auto func = [&io_context](){io_context.run();};
	std::thread t(func);
	char line[512];
	std::string s;
	while(std::cin >> s)
	{	
		if (!s.size())
		{
			continue;
		}
		std::cout << s << std::endl;
		c.write(s);
	}
	t.join();
	
	std::cout<<"hello world"<<std::endl;
}