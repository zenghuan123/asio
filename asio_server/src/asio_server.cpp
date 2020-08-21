#include "asio_server.h"
#include <iostream>
std::string asio_server::gen_id()
{
	clock_t t = clock_t();
	return std::to_string(t);
}
void asio_server::TcpClient::write(const std::string & message)
{
	Message m;
	m.body_length(message.size());
	m.encode_header();
	strcpy_s(m.body(),m.max_body_length,message.c_str());
	
	asio::post(io_context_, std::bind(&asio_server::TcpClient::do_write,this, m));
};
void asio_server::TcpClient::do_write(const Message& message)
{
	bool need_process = write_msg_queue_.empty();
	write_msg_queue_.push(message);
	if(need_process)
	{	
		Message m = write_msg_queue_.front();
		asio::async_write(socket_,asio::buffer(m.data(),m.length()),std::bind(&asio_server::TcpClient::handle_write,this,std::placeholders::_1));
	}
}
void asio_server::TcpClient::handle_write(const asio::error_code&error)
{
	if(!error)
	{
		write_msg_queue_.pop();
		if(!write_msg_queue_.empty())
		{	
			Message m = write_msg_queue_.front();
			asio::async_write(socket_,asio::buffer(m.data(),m.length()),std::bind(&asio_server::TcpClient::handle_write,this,std::placeholders::_1));
		}
	}else{
		std::cout<<"write fail"<<std::endl;
	}
}

void asio_server::TcpClient::close()
{

};
void asio_server::TcpClient::do_close()
{

}

void asio_server::TcpClient::connect()
{
	asio::post(io_context_,std::bind(&asio_server::TcpClient::do_connect,this));
	
};
void asio_server::TcpClient::do_connect()
{
	asio::async_connect(this->socket_,this->end_point_, std::bind(&asio_server::TcpClient::handle_connect,this,std::placeholders::_1));
}

void asio_server::TcpClient::handle_connect(const asio::error_code& error)
{
	if(!error)
	{
		asio::async_read(socket_, asio::buffer(read_msg_.data(),Message::header_length),std::bind(&asio_server::TcpClient::handle_read_head,this,std::placeholders::_1));

	}else{
		std::cout<<"connect fail"<<std::endl;
	}
}
void asio_server::TcpClient::handle_read_head(const asio::error_code&error)
{
	if(!error)
	{
		read_msg_.decode_header();
		asio::async_read(socket_,asio::buffer(read_msg_.body(),read_msg_.body_length()),std::bind(&asio_server::TcpClient::handle_read_body,this,std::placeholders::_1));
	}else{
		std::cout<<"read head fail"<<std::endl;
	}
}
void asio_server::TcpClient::handle_read_body(const asio::error_code& error)
{
	if(!error)
	{
			auto ptr = read_msg_.body();
			std::string  message = std::string(ptr, ptr+read_msg_.body_length());
			this->handle_message(message);
	}
}

void asio_server::TcpClient::handle_message(const std::string&message)
{
	std::cout<<"client receive"<<message<<std::endl;
}
