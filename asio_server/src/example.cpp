
#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

class chat_message
{
public:
	enum { header_length = 4 };
	enum { max_body_length = 512 };

	chat_message()
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
		using namespace std; // For strncat and atoi.
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
		using namespace std; // For sprintf and memcpy.
		char header[header_length + 1] = "";
		sprintf(header, "%4d", static_cast<int>(body_length_));
		memcpy(data_, header, header_length);
	}

private:
	char data_[header_length + max_body_length];
	size_t body_length_;
};


//---------------------------------------------------------------------

//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "asio.hpp"
#include <deque>
#include <set>
#include <memory>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>

using asio::ip::tcp;


//----------------------------------------------------------------------
typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class chat_participant
{
public:
	virtual ~chat_participant() {}
	virtual void deliver(const chat_message& msg) = 0;
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room
{
public:
	void join(chat_participant_ptr participant)
	{
		participants_.insert(participant);
		std::for_each(recent_msgs_.begin(), recent_msgs_.end(),
			std::bind(&chat_participant::deliver,
				participant, std::placeholders::_1));
	}

	void leave(chat_participant_ptr participant)
	{
		participants_.erase(participant);
	}

	void deliver(const chat_message& msg)
	{
		recent_msgs_.push_back(msg);
		while (recent_msgs_.size() > max_recent_msgs)
			recent_msgs_.pop_front();

		std::for_each(participants_.begin(), participants_.end(),
			std::bind(&chat_participant::deliver,
				std::placeholders::_1, std::ref(msg)));
	}

private:
	std::set<chat_participant_ptr> participants_;
	enum { max_recent_msgs = 100 };
	chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class chat_session
	: public chat_participant,
	public std::enable_shared_from_this<chat_session>
{
public:
	chat_session(asio::io_context& io_context, chat_room& room)
		: socket_(io_context),
		room_(room)
	{
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		room_.join(shared_from_this());
		asio::async_read(socket_,
			asio::buffer(read_msg_.data(), chat_message::header_length),
			std::bind(
				&chat_session::handle_read_header, shared_from_this(),
				std::placeholders::_1));
	}

	void deliver(const chat_message& msg)
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			asio::async_write(socket_,
				asio::buffer(write_msgs_.front().data(),
					write_msgs_.front().length()),
				std::bind(&chat_session::handle_write, shared_from_this(),
					std::placeholders::_1));
		}
	}

	void handle_read_header(const asio::error_code& error)
	{
		if (!error && read_msg_.decode_header())
		{
			asio::async_read(socket_,
				asio::buffer(read_msg_.body(), read_msg_.body_length()),
				std::bind(&chat_session::handle_read_body, shared_from_this(),
					std::placeholders::_1));
		}
		else
		{
			room_.leave(shared_from_this());
		}
	}

	void handle_read_body(const asio::error_code& error)
	{
		if (!error)
		{
			room_.deliver(read_msg_);
			asio::async_read(socket_,
				asio::buffer(read_msg_.data(), chat_message::header_length),
				std::bind(&chat_session::handle_read_header, shared_from_this(),
					std::placeholders::_1));
		}
		else
		{
			room_.leave(shared_from_this());
		}
	}

	void handle_write(const asio::error_code& error)
	{
		if (!error)
		{
			write_msgs_.pop_front();
			if (!write_msgs_.empty())
			{
				asio::async_write(socket_,
					asio::buffer(write_msgs_.front().data(),
						write_msgs_.front().length()),
					std::bind(&chat_session::handle_write, shared_from_this(),
						std::placeholders::_1));
			}
		}
		else
		{
			room_.leave(shared_from_this());
		}
	}

private:
	tcp::socket socket_;
	chat_room& room_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
};

typedef std::shared_ptr<chat_session> chat_session_ptr;

//----------------------------------------------------------------------

class chat_server
{
public:
	chat_server(asio::io_context& io_context,
		const tcp::endpoint& endpoint)
		: io_context_(io_context),
		acceptor_(io_context, endpoint)
	{
		start_accept();
	}

	void start_accept()
	{
		chat_session_ptr new_session(new chat_session(io_context_, room_));
		acceptor_.async_accept(new_session->socket(),
			std::bind(&chat_server::handle_accept, this, new_session,
				std::placeholders::_1));
	}

	void handle_accept(chat_session_ptr session,
		const asio::error_code& error)
	{
		if (!error)
		{
			session->start();
		}

		start_accept();
	}

private:
	asio::io_context& io_context_;
	tcp::acceptor acceptor_;
	chat_room room_;
};

typedef std::shared_ptr<chat_server> chat_server_ptr;
typedef std::list<chat_server_ptr> chat_server_list;

//----------------------------------------------------------------------



void server_run()
{
	try
	{

		asio::io_context io_context;

		chat_server_list servers;
		int argc = 2;
		for (int i = 1; i < argc; ++i)
		{
			using namespace std; // For atoi.
			tcp::endpoint endpoint(tcp::v4(),8888);
			chat_server_ptr server(new chat_server(io_context, endpoint));
			servers.push_back(server);
		}

		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

}
//--------------------------------------------------------------------------------------------------------

//
// chat_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include "asio.hpp"

using asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
	chat_client(asio::io_context& io_context,
		const tcp::resolver::results_type& endpoints)
		: io_context_(io_context),
		socket_(io_context)
	{
		asio::async_connect(socket_, endpoints,
			std::bind(&chat_client::handle_connect, this,
				std::placeholders::_1));
	}

	void write(const chat_message& msg)
	{
		asio::post(io_context_,
			std::bind(&chat_client::do_write, this, msg));
	}

	void close()
	{
		asio::post(io_context_,
			std::bind(&chat_client::do_close, this));
	}

private:

	void handle_connect(const asio::error_code& error)
	{
		if (!error)
		{
			asio::async_read(socket_,
				asio::buffer(read_msg_.data(), chat_message::header_length),
				std::bind(&chat_client::handle_read_header, this,
					std::placeholders::_1));
		}
	}

	void handle_read_header(const asio::error_code& error)
	{
		if (!error && read_msg_.decode_header())
		{
			asio::async_read(socket_,
				asio::buffer(read_msg_.body(), read_msg_.body_length()),
				std::bind(&chat_client::handle_read_body, this,
					std::placeholders::_1));
		}
		else
		{
			do_close();
		}
	}

	void handle_read_body(const asio::error_code& error)
	{
		if (!error)
		{
			std::cout.write(read_msg_.body(), read_msg_.body_length());
			std::cout << "\n";
			asio::async_read(socket_,
				asio::buffer(read_msg_.data(), chat_message::header_length),
				std::bind(&chat_client::handle_read_header, this,
					std::placeholders::_1));
		}
		else
		{
			do_close();
		}
	}

	void do_write(chat_message msg)
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			asio::async_write(socket_,
				asio::buffer(write_msgs_.front().data(),
					write_msgs_.front().length()),
				std::bind(&chat_client::handle_write, this,
					std::placeholders::_1));
		}
	}

	void handle_write(const asio::error_code& error)
	{
		if (!error)
		{
			write_msgs_.pop_front();
			if (!write_msgs_.empty())
			{
				asio::async_write(socket_,
					asio::buffer(write_msgs_.front().data(),
						write_msgs_.front().length()),
					std::bind(&chat_client::handle_write, this,
						std::placeholders::_1));
			}
		}
		else
		{
			do_close();
		}
	}

	void do_close()
	{
		socket_.close();
	}

private:
	asio::io_context& io_context_;
	tcp::socket socket_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
};


void client_run()
{
	try
	{


		asio::io_context io_context;

		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "8888");

		chat_client c(io_context, endpoints);
		
		auto func = [&io_context]() {io_context.run(); };

		asio::thread t(func);

		char line[chat_message::max_body_length + 1];
		while (std::cin.getline(line, chat_message::max_body_length + 1))
		{
			using namespace std; // For strlen and memcpy.
			chat_message msg;
			msg.body_length(strlen(line));
			memcpy(msg.body(), line, msg.body_length());
			msg.encode_header();
			c.write(msg);
		}

		c.close();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

}
#endif // CHAT_MESSAGE_HPP