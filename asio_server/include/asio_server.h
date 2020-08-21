#ifndef ASIO_SERVER
#pragma once
#define ASIO_SERVER
#include <string>
#include <ctime>
#include <memory>
#include <queue>
#include <asio.hpp>
#include<iostream>

namespace asio_server{
using asio::ip::tcp;
std::string gen_id();


class TcpSession;
typedef std::shared_ptr<TcpSession> tcp_session_ptr;

class SessionManager
{
public:
	tcp_session_ptr get_session(std::string id)
	{
		auto it = session_map_.find(id);
		if (it == session_map_.end())
		{
			return it->second;
		}
		else {
			return NULL;
		}
	}
	void add_session(std::string id, tcp_session_ptr ptr)
	{
		session_map_[id] = ptr;
	}
	void remove_session(std::string id)
	{
		session_map_.erase(id);
	}
	SessionManager() {};

private:
	
	std::unordered_map<std::string, tcp_session_ptr> session_map_;
};

class Message
{
public:
	enum { header_length = 4 };
	enum { max_body_length = 512 };

	Message()
		: body_length_(0)
	{
	}

	const char* data() const
	{
		return data_;
	}

	char* data()
	{
		return data_;
	}

	size_t length() const
	{
		return header_length + body_length_;
	}

	const char* body() const
	{
		return data_ + header_length;
	}

	char* body()
	{
		return data_ + header_length;
	}

	size_t body_length() const
	{
		return body_length_;
	}

	void body_length(size_t new_length)
	{
		body_length_ = new_length;
		if (body_length_ > max_body_length)
			body_length_ = max_body_length;
	}

	bool decode_header()
	{
		using namespace std; 
		char header[header_length + 1] = "";
		strncat(header, data_, header_length);
		body_length_ = atoi(header);
		if (body_length_ > max_body_length)
		{
			body_length_ = 0;
			return false;
		}
		return true;
	}

	void encode_header()
	{
		using namespace std; 
		char header[header_length + 1] = "";
		sprintf(header, "%4d", static_cast<int>(body_length_));
		memcpy(data_, header, header_length);
	}

private:
	char data_[header_length + max_body_length];
	size_t body_length_;
};

class TcpSession:public std::enable_shared_from_this<TcpSession>
{
public:
	TcpSession(asio::io_context&io_context, SessionManager& manager):socket_(io_context),id_(gen_id()),manager_(manager)
	{
		
	}

	tcp::socket& socket()
	{
		return socket_;
	
	}
	void start()
	{
		this->manager_.add_session(this->id(), shared_from_this());
		this->start_read_head();
	}

	void disconnect()
	{
		this->manager_.remove_session(this->id());
	}

	std::string id()
	{
		return id_;
	}
	virtual void handle_message(const std::string& message)
	{
		std::cout<<"server receive"<<message<<std::endl;
	}


private:
	void start_read_head()
	{
		asio::async_read(socket_,asio::buffer(message_.data(), Message::header_length), std::bind(&TcpSession::read_header, shared_from_this(),std::placeholders::_1));
	}
	void read_header(const asio::error_code& error)
	{
		std::cout << "read_header" << std::endl;
		if (!error)
		{
			if (message_.decode_header())
			{
				asio::async_read(socket_,
					asio::buffer(message_.body(), message_.body_length()), 
					std::bind(&TcpSession::read_body,shared_from_this(),std::placeholders::_1)
				);
			}
			else {
				this->disconnect();
			}
		}
		else {
			this->disconnect();

		}
	}
	void read_body(const asio::error_code&error)
	{	
		
		std::cout << "read_body" << std::endl;
		if (!error)
		{	
			this->start_read_head();
			auto ptr = message_.body();
			std::string  message = std::string(ptr, ptr+message_.body_length());
			this->handle_message(message);
		}
	}

	tcp::socket socket_;
	Message message_;
	std::string id_;
	SessionManager manager_;
};


class TcpServer
{
	;
public:
	TcpServer(asio::io_context& io_context,std::string ip, int port) :io_context_(io_context), accpetor_(io_context, tcp::endpoint(asio::ip::make_address(ip), port))
	{
	
	}
	void start_accpet()
	{
		tcp_session_ptr new_session = tcp_session_ptr(new TcpSession(io_context_, manager_));
		accpetor_.async_accept(new_session->socket(),
			std::bind(&TcpServer::handle_accept, this, new_session,
				std::placeholders::_1));

	}
private:

	void handle_accept(tcp_session_ptr session,
		const asio::error_code& error)
	{	
		std::cout << "handle_accept" << std::endl;
		start_accpet();
		if (!error)
		{
			session->start();
		}
	}



	asio::io_context& io_context_;
	tcp::acceptor accpetor_;
	SessionManager manager_;
};


class TcpClient
{
public:
	TcpClient(asio::io_context& io_context, std::string ip, int port):io_context_(io_context),socket_(io_context)
	{
		tcp::resolver resolver(io_context);
		end_point_ = resolver.resolve(ip, std::to_string(port));

	}
	void write(const std::string& message);
	void close();
	void connect();
	virtual void handle_message(const std::string& message);

private:
	void do_connect();
	void handle_connect(const asio::error_code& error);
	void do_write(const Message& message);
	void handle_write(const asio::error_code&error);
	void do_close();
	void handle_read_head(const asio::error_code &error);
	void handle_read_body(const asio::error_code &error);
	

	asio::io_context& io_context_;
	tcp::socket socket_;
	tcp::resolver::results_type end_point_;
	Message read_msg_;
	std::queue<Message> write_msg_queue_;
	
};


}

#endif