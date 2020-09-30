#include<iostream>
#include "example.h"
#include "asio_server.h"
using namespace asio_server;
typedef std::shared_ptr<TcpServer> server_ptr;
int id_ = 0;
void new_msg(asio_server::TcpConnection::ptr ptr, const std::string &msg)
{
	std::cout << "recieve from " << ptr->id() << ":" << msg << std::endl;
}

void new_connection_callback(server_ptr servers, asio_server::TcpConnection::ptr ptr)
{
	ptr->set_read_callback(std::bind(&new_msg,ptr,std::placeholders::_1));
	
	ptr->write(asio_server::Message::parse_from_str(std::to_string(id_)));
	id_ += 1;
}
int main(int argc, char* argv[])
{
	//server_run();
	using namespace asio::ip;
	//server_run();
	asio::io_context io_context;
	
	server_ptr ptr(new TcpServer(io_context, "127.0.0.1", 999999));
	//asio_server::TcpServer *server_ptr = new asio_server::TcpServer(io_context,"127.0.0.1",999999);
	ptr->start_accpet();
	ptr->set_new_connection(std::bind(new_connection_callback, ptr, std::placeholders::_1));
	io_context.run();
	std::cout<<"hello world"<<std::endl;
}

