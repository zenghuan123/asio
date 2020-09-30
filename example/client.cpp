#include<iostream>
#include "example.h"
#include "asio_server.h"
#include <thread>
#include <string>
std::string name = "";
typedef std::shared_ptr<asio_server::TcpClient> s_ptr;
void new_msg(s_ptr ptr, const std::string &msg)
{
	if (name.length() == 0)
	{
		name = msg;
	}
	std::cout << "recieve from server" << ":" << msg << std::endl;
}

int main(int argc, char* argv[])
{

	//client_run();
	asio::io_context io_context;
	s_ptr c(new asio_server::TcpClient(io_context, "127.0.0.1", 999999));
	c->set_read_callback(std::bind(new_msg, c, std::placeholders::_1));
	c->connect();
	io_context.run();
	auto func = [&io_context](){io_context.run();};
	asio::thread t(func);
	char line[512];

	while(std::cin.getline(line, 513))
	{	
		std::string s = line;//		s = std::string(line);
		if (!s.size())
		{
			continue;
		}
		std::cout << s << std::endl;
		s = name + s;
		c->write_async(s);
	}
	t.join();
	
	std::cout<<"hello world"<<std::endl;
}