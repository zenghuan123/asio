#include "asio_server.h"
#include <iostream>
std::string asio_server::gen_id()
{
	clock_t t = clock_t();
	return std::to_string(t);
}


asio_server::TcpConnection::TcpConnection(asio::io_context & io_context, std::string id):MessageHandle(io_context), id_(id)
{
	std::cout << "OK" << std::endl;
}



asio_server::TcpServer::TcpServer(asio::io_context& io_context, std::string ip, int port) 
	:io_context_(io_context), accpetor_(io_context, tcp::endpoint(asio::ip::tcp::v4(), port))
{

}

void asio_server::TcpServer::set_new_connection(std::function<void(TcpConnection::ptr)> callback)
{
	new_connection_callback_ = callback;
}
void asio_server::TcpServer::message_to_all(const std::string &message)
{
	for (auto it : manager_.id_to_connection_)
	{
		auto ptr = it.second.lock();
		if (ptr)
		{
			Message msg;

			msg.body_length(message.size());
			memcpy(msg.body(), message.c_str(), msg.body_length());
			msg.encode_header();
			ptr->write(msg);
		}
	}
}

void asio_server::TcpServer::handle_accept(TcpConnection::ptr new_connection, const asio::error_code & error)
{
	
		std::cout << "handle_accept" << std::endl;
		start_accpet();
		if (!error)
		{
			//	session->start();
			manager_.add_connection(new_connection);
			auto f =std::bind(&TcpServer::on_connection_close, this,new_connection->id());
			new_connection->set_close_callback(f);
			new_connection_callback_(new_connection);
		}
	
}

void asio_server::TcpServer::on_connection_close(std::string id)
{
	manager_.remove_connection(id);
}

// ----------------------TcpClient

asio_server::TcpClient::TcpClient(asio::io_context& io_context, std::string ip, int port):MessageHandle(io_context)
{
	tcp::resolver resolver(io_context);
	end_point_ = resolver.resolve(ip, std::to_string(port));

}

void asio_server::TcpClient::write_async(const std::string& message)
{
	Message msg;
	
	msg.body_length(message.size());
	memcpy(msg.body(), message.c_str(), msg.body_length());
	msg.encode_header();
	asio::post(std::bind(&asio_server::TcpClient::write, shared_from_this(), msg));
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
		asio::async_read(socket_, asio::buffer(read_msg_.data(),Message::header_length),std::bind(&asio_server::TcpClient::read_head,this,std::placeholders::_1));

	}else{
		std::cout<<"connect fail"<<std::endl;
	}
}

//---------------------------------------

asio_server::MessageHandle::MessageHandle(asio::io_context& io_context):io_context_(io_context),socket_(io_context_)
{
	std::cout << "message handle" << std::endl;
}
asio_server::MessageHandle::~MessageHandle()
{
	close();
	socket_.close();
}
void asio_server::MessageHandle::write(const Message& message)
{
	bool need_process = write_msg_queue_.empty();
	write_msg_queue_.push(message);
	if (need_process)
	{
		std::cout << "write" << std::endl;
		Message m = write_msg_queue_.front();
		asio::async_write(socket_, asio::buffer(m.data(), m.length()), std::bind(&asio_server::MessageHandle::handle_write, this, std::placeholders::_1));
	}
}
void asio_server::MessageHandle::close()
{
	close_callback_();
}
void asio_server::MessageHandle::set_read_callback(std::function<void(const std::string&msg)>callback)
{
	read_callback_ = callback;
}
void asio_server::MessageHandle::set_close_callback(std::function<void(void)> callback_)
{
	close_callback_ = callback_;
}
void asio_server::MessageHandle::handle_write(const asio::error_code&error)
{
	if (!error)
	{
		write_msg_queue_.pop();
		if (!write_msg_queue_.empty())
		{
			Message m = write_msg_queue_.front();
			asio::async_write(socket_, asio::buffer(m.data(), m.length()), std::bind(&asio_server::MessageHandle::handle_write, this, std::placeholders::_1));
		}
	}
	else {
		std::cout << "write fail" << std::endl;
	}
}

void asio_server::MessageHandle::read_head(const asio::error_code&error)
{
	if (!error)
	{
		read_msg_.decode_header();
		asio::async_read(socket_, asio::buffer(read_msg_.body(), read_msg_.body_length()), std::bind(&asio_server::MessageHandle::read_body, this, std::placeholders::_1));
	}
	else {
		std::cout << "read head fail" << std::endl;
	}
}
void asio_server::MessageHandle::read_body(const asio::error_code& error)
{
	if (!error)
	{
		auto ptr = read_msg_.body();
		std::string  message = std::string(ptr, ptr + read_msg_.body_length());
		read_callback_(message);
	}
	else {
		std::cout << "read body fail" << std::endl;
	}
}

