#ifndef ASIO_SERVER
#pragma once
#define ASIO_SERVER
#include <string>
#include <ctime>
#include <memory>
#include <queue>
#include <asio.hpp>
#include<iostream>
#include<unordered_map>

namespace asio_server{
using asio::ip::tcp;
std::string gen_id();

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

	static Message parse_from_str(const std::string&msg)
	{
		Message m;
		m.body_length(msg.size());
		m.encode_header();
		strcpy_s(m.body(), m.max_body_length, msg.c_str());
		return m;
	}



private:
	char data_[header_length + max_body_length];
	size_t body_length_;
};
class TcpServer;
class MessageHandle:public std::enable_shared_from_this<MessageHandle>
{
public:
	
	 MessageHandle(asio::io_context& io_context );
	virtual ~MessageHandle();
	void write(const Message&message);
	void close();
	void set_read_callback(std::function<void(const std::string &msg)>);
	void set_close_callback(std::function<void(void)> callback_);
protected:
	
	void handle_write(const asio::error_code&error);
	void read_head(const asio::error_code&error);
	void read_body(const asio::error_code&error);
	
	asio::io_context& io_context_;
	tcp::socket socket_;
	
	Message read_msg_;
	std::queue<Message> write_msg_queue_;
	std::function<void(const std::string &msg)> read_callback_;
	std::function<void(void)> close_callback_;
};


class TcpConnection:public MessageHandle
{
	
public:
	typedef std::shared_ptr<TcpConnection> ptr;

	TcpConnection(asio::io_context& io_context, std::string _id = gen_id());
	inline tcp::socket& socket()
	{
		return this->socket_;
	}
	inline const std::string& id() {
		return id_;
	}
private:
	std::string id_;

};
class TcpConnectionManager
{
public:
	friend class TcpServer;
	inline void add_connection(TcpConnection::ptr ptr)
	{
		id_to_connection_[ptr->id()] = ptr;
	}
	inline void remove_connection(std::string id)
	{
		id_to_connection_.erase(id);
	}
	inline TcpConnection::ptr get_connection(std::string id)
	{
		auto it = id_to_connection_.find(id);
		if (it != id_to_connection_.end())
		{
			return it->second.lock();
		}
		else {
			return NULL;
		}
	}
private:
	std::unordered_map<std::string, std::weak_ptr<TcpConnection>> id_to_connection_;

};

class TcpServer
{
	
public:
	TcpServer(asio::io_context& io_context, std::string ip, int port);
	void start_accpet()
	{
		auto p = new TcpConnection(io_context_);
		
		TcpConnection::ptr new_connection(p);

		accpetor_.async_accept(new_connection->socket(),
			std::bind(&TcpServer::handle_accept, this, new_connection,
				std::placeholders::_1));
	}
	void set_new_connection(std::function<void(TcpConnection::ptr)> call_back);
	inline const TcpConnectionManager& get_connection_manager()
	{
		return manager_;
	}

	void message_to_all(const std::string &msg);

private:

	void handle_accept(TcpConnection::ptr new_connection, const asio::error_code& error);

	void on_connection_close(std::string id_);

	asio::io_context& io_context_;
	tcp::acceptor accpetor_;
	TcpConnectionManager manager_;
	std::function<void(TcpConnection::ptr)> new_connection_callback_;
};



class TcpClient:public MessageHandle
{
public:
	TcpClient(asio::io_context& io_context, std::string ip, int port);
	void write_async(const std::string& message);
	
	void connect();

private:
	void do_connect();
	void handle_connect(const asio::error_code& error);	
	tcp::resolver::results_type end_point_;
};


}

#endif