#include<iostream>
#include "example.h"
#include "asio_server.h"
int main(int argc, char* argv[])
{
	//server_run();
	asio::io_context io_context;
	asio_server::TcpServer server(io_context,"127.0.0.1",999999);
	server.start_accpet();
	io_context.run();
	std::cout<<"hello world"<<std::endl;
}