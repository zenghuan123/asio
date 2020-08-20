#ifndef ASIO_SERVER
#define ASIO_SERVER
#include <string>
#include <ctime>
#include <memory>
#include <asio.hpp>
namespace asio_server{

std::string gen_id()
{
	clock_t t = clock_t();
	return std::to_string(t);
}

class tcp_session;
typedef std::shared_ptr<tcp_session> tcp_session_ptr;

class SessionManager
{
public:
	static tcp_session_ptr get_session(std::string id)
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
	static void add_session(std::string id, tcp_session_ptr ptr)
	{
		session_map_[id] = ptr;
	}
	static void remove_session(std::string id)
	{
		session_map_.erase(id);
	}

private:
	SessionManager() {}
	static std::unordered_map<std::string, tcp_session_ptr> session_map_;
};

class tcp_session:public std::enable_shared_from_this<tcp_session>
{
public:
	tcp_session(asio::io_context&io_context):socket_(io_context),id_(gen_id())
	{
		SessionManager::add_session(id_, tcp_session_ptr(this));
	}

	tcp::socket& socket()
	{
		return socket_;
	
	}
	void start()
	{
		this->start_read_head();
		this -> on_connected();
	}
	virtual void on_connected()
	{	
		//连接上了
		
	}
	virtual void on_disconnected()
	{
		//断开连接
	}
	std::string id()
	{
		return id_;
	}



	void disconnect()
	{
		SessionManager::remove_session(id_);
		this->on_disconnected();
	}



private:
	virtual ~tcp_session()
	{
		
	}

	void start_read_head()
	{
		asio::async_read(socket_,asio::buffer(message_.data(), Message::header_length), std::bind(&tcp_session::read_header, shared_from_this(),std::placeholders::_1));
	}
	void read_header(const asio::error_code& error)
	{
		
		if (!error)
		{
			if (message_.decode_header())
			{
				asio::async_read(socket_,
					asio::buffer(message_.body(), message_.body_length()), 
					std::bind(&tcp_session::read_body,shared_from_this(),std::placeholders::_1)
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
		if (!error)
		{
			std::string s = 
		}
	}

	tcp::socket socket_;
	Message message_;
	std::string id_;
	bool has_connected_;
};


}

#endif