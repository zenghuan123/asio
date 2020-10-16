#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include<thread>
#include<mutex>
#include<queue>

#include<unordered_map>

std::string print(struct sockaddr_in * serveraddr)
{
	char buff[128];
	inet_ntop(AF_INET, &(serveraddr->sin_addr), buff, (socklen_t)(sizeof(buff)));//ip二进制转字符串
	uint16_t port = ntohs(serveraddr->sin_port);
	//int end = strlen(buff);
	//snprintf(buff + end, 128 - end, ":%u", port);
	std::cout << buff <<":"<< port << std::endl;
	return std::string(buff);
}

void server(char*argv[])
{
	int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);

	struct sockaddr_in serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;

	char * local_addr = argv[1];
	inet_pton(AF_INET, local_addr, &(serveraddr.sin_addr));//ip字符串转二进制

	char buff[16];
	inet_ntop(AF_INET, &(serveraddr.sin_addr), buff, (socklen_t)(sizeof(buff)));//ip二进制转字符串
	std::cout << "ip" << buff << std::endl;


	//inet_aton(local_addr, &(serveraddr.sin_addr));
	int portnumber = atoi(argv[2]);
	serveraddr.sin_port = htons(portnumber);//小端转大端

	bind(listenfd, (sockaddr*)&serveraddr, sizeof(serveraddr));
	listen(listenfd, 10);

	struct epoll_event events[20];

	struct epoll_event ev;
	ev.data.fd = listenfd;
	
	ev.events = EPOLLIN;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listenfd, &ev);

	//int std_in_fd = 0;
	//ev.data.fd = std_in_fd;
	//ev.events = EPOLLIN;
	//epoll_ctl(epoll_fd, EPOLL_CTL_ADD, std_in_fd, &ev);

	std::unordered_map<std::string, int> address_to_fd;

	while (true)
	{
		std::cout << "epoll_wait start" << std::endl;
		int nfds = epoll_wait(epoll_fd, events, 20, 10000);
		std::cout << "epoll_wait end" << std::endl;
		for (int i = 0; i < nfds; ++i)
		{
			//这里直接遍历的所有有事件的fd,并且直接就处理了
			//muduo是把fd和对应的事件先记下来等这个循环结束后再处理
			if (events[i].data.fd == listenfd)
			{
				struct sockaddr_in clientaddr;
				socklen_t clilent;

				int connfd = accept4(listenfd, (sockaddr*)&clientaddr, &clilent, SOCK_NONBLOCK | SOCK_CLOEXEC);
				char *str = inet_ntoa(clientaddr.sin_addr);

				std::cout << "accapt a connect" << str <<"fd is"<< connfd<< std::endl;
				ev.data.fd = connfd;
				ev.events = EPOLLIN;
				struct sockaddr_in localaddr;
				socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
				if (getsockname(connfd, (sockaddr*)(&localaddr), &addrlen) < 0)//获得本地地址
				{
					std::cout << "getsockname error" << std::endl;
				}
				else {
					std::cout << "get sockname" << " ";
					print(&localaddr);
				}
				std::cout << "client addr" << " ";
				print(&clientaddr);
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &ev);
			}
			else
			{ 
				
				if (events[i].events&EPOLLHUP & !(events[i].events&EPOLLIN))
				{
					//没跑出来
					int socket_fd = events[i].data.fd;
					std::cout << "EPOLLHUP close " << "fd" << socket_fd<< std::endl;
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, &ev);//ev可以为NULl,但是在内核2.6.9以前会出错

				}
				if (events[i].events && (EPOLLERR))
				{
					int optval;
					socklen_t optlen = static_cast<socklen_t>(sizeof optval);
					//获取一个套接字的各种数据
					int error = getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);

					//EINPROGRESS  Operation now in progress
					std::cout << " errno:" << errno << "  socket is error? " << (error < 0 ? "true" : "false") << std::endl;
				}

				if (events[i].events&(EPOLLIN | EPOLLPRI | EPOLLRDHUP))
				{
					std::cout << "read data" << std::endl;
					int socket_fd = events[i].data.fd;
					struct iovec vec[2];
					char buff1[16];
					bzero(buff1, sizeof buff1);
					char buff2[1024];
					bzero(buff2, sizeof buff2);
					vec[0].iov_base = buff1;
					vec[0].iov_len = sizeof buff1;
					vec[1].iov_base = buff2;
					vec[1].iov_len = sizeof buff2;
					ssize_t n = readv(socket_fd, vec, 2);
					if (n == 0)
					{
						//ctrl +c , kill，主动close(fd)
						std::cout << "client close " << errno << std::endl;
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, &ev);
						continue;
					}else
					if (n < 0)
					{
						std::cout << "read error " << errno << std::endl;
						//epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, &ev);
						continue;
					}
					std::cout << buff1 << std::endl;
					std::cout << buff2 << std::endl;


					char receive_msg[] = " recieve msg  success ";
					size_t length = strlen(receive_msg);
					n = write(socket_fd, receive_msg, length);
					if (n < 0)
					{
						std::cout << "send error" << errno << std::endl;
						//epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, &ev);
						continue;
					}
					else
					{
						if (length - n > 0)
						{
							ev.data.fd = socket_fd;
							ev.events = EPOLLIN  | EPOLLOUT;
							//发送数据有剩余，需要监听 可写事件
							//理论上应该先判断之前有没有监听可写事件，如果有就不需要再设置
							epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &ev);
						}
						else {
							//没有剩余，需要判断一下有没有监听可写事件
							//有监听的情况下取消设置,不取消的话，epo
							ev.data.fd = socket_fd;
							ev.events = EPOLLIN;
							//发送数据有剩余，需要监听 可写事件
							//理论上应该先判断之前有没有监听可写事件，如果有就不需要再设置
							epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &ev);
						}
					}

				}
				if (events[i].events&EPOLLOUT)
				{


					//每次发送基本都要检查
					std::cout << "can send data" << std::endl;
					int socket_fd = events[i].data.fd;
					ev.data.fd = socket_fd;
					ev.events = EPOLLIN;
					
					epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &ev);
				}
			}

		}
	}
}
void client(char*argv[])
{
	
	int epoll_fd = epoll_create(256);
	int client_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	struct sockaddr_in serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;

	char *local_addr = argv[1];
	inet_pton(AF_INET, local_addr, &(serveraddr.sin_addr));//ip字符串转二进制

	char buff[16];
	inet_ntop(AF_INET, &(serveraddr.sin_addr), buff, (socklen_t)(sizeof(buff)));//ip二进制转字符串
	std::cout << "ip" << buff << std::endl;

	struct epoll_event ev;
	ev.data.fd = client_fd;
	ev.events = EPOLLOUT ;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

	ev.data.fd = evtfd;
	ev.events = EPOLLIN;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, evtfd, &ev);

	//inet_aton(local_addr, &(serveraddr.sin_addr));

	int portnumber = atoi(argv[2]);
	serveraddr.sin_port = htons(portnumber);//小端转大端

	int error = connect(client_fd, (struct sockaddr*)(&serveraddr), static_cast<socklen_t>(sizeof(serveraddr)));
	if (error<0 && errno!= EINPROGRESS)
	{
		std::cout << "connect error" << error << std::endl;
		exit(1);
		return;
	}
	std::queue<std::string> send_msg;
	std::mutex m;

	std::thread t([evtfd,&send_msg,&m]() {
		std::cout << "event fd"<< evtfd << std::endl;
		std::string line;
		uint64_t i = 1;

		while(std::getline(std::cin, line))
		{
			m.lock();
			send_msg.push(line);
			m.unlock();
			write(evtfd, &i, sizeof i);
		}
	});

	struct epoll_event events[20];


	
	bool has_connect = false;
	while (true)
	{
		std::cout << "epoll_wait start" << std::endl;
		int nfds = epoll_wait(epoll_fd, events, 20, 10000);
		std::cout << "epoll_wait end" << std::endl;
		
		for (int i = 0; i < nfds; ++i)
		{
			
				if (events[i].events&EPOLLHUP & !(events[i].events&EPOLLIN))
				{
					int fd = events[i].data.fd;
					std::cout << "EPOLLHUP  close " << "fd" << std::endl;
					
					exit(1);
				}

				if (events[i].events&(EPOLLIN | EPOLLPRI | EPOLLRDHUP))
				{					
					int socket_fd = events[i].data.fd;
					if (socket_fd == evtfd)
					{
						std::cout << "wake up" << std::endl;
						uint64_t i;
						const ssize_t n = read(socket_fd, &i, sizeof i);
						m.lock();
						std::string line = send_msg.front();
						if (line=="close")
						{
							std::cout << "try to close connect " << std::endl;
							close(client_fd);
							exit(1);
							continue;
						}else if(line=="shutdown")
						{
							std::cout << "try to shutdown connect " << std::endl;
							shutdown(client_fd, SHUT_RDWR);
							exit(1);
						}
						send_msg.pop();
						m.unlock();
						write(client_fd, line.c_str(), line.size());
						continue;
					}
					std::cout << "read data" << std::endl;
					struct iovec vec[2];
					char buff1[16];
					bzero(buff1, sizeof buff1);
					char buff2[1024];
					bzero(buff2, sizeof buff2);
					vec[0].iov_base = buff1;
					vec[0].iov_len = sizeof buff1;
					vec[1].iov_base = buff2;
					vec[1].iov_len = sizeof buff2;
					const ssize_t n = readv(socket_fd, vec, 2);
					if (n == 0)
					{
						//
						//对面关了触发读事件？？
						std::cout <<" "<< (events[i].events&&EPOLLIN )<<" :"<< (events[i].events&& EPOLLPRI) <<":"<< (events[i].events&& EPOLLRDHUP) << std::endl;
						std::cout << "close" << std::endl;
						exit(1);
					}else if (n < 0)
					{
						std::cout << "read error " << errno << std::endl;
						//什么情况跑到这
						exit(1);
					}
					std::cout << buff1 << std::endl;
					std::cout << buff2 << std::endl;

				}
				if (events[i].events&EPOLLOUT)
				{
					
					if (events[i].data.fd == client_fd)
					{
						if (has_connect)
						{

						}
						else {
							has_connect = true;
							
							std::cout << "connect success can send data" << std::endl;
							struct sockaddr_in localaddr;
							memset(&localaddr, 0, sizeof localaddr);
							socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
							getsockname(client_fd, (struct sockaddr*)(&localaddr), &addrlen);

							struct sockaddr_in peeraddr;
							memset(&peeraddr, 0, sizeof peeraddr);
							socklen_t peeraddrlen = static_cast<socklen_t>(sizeof peeraddr);
							getpeername(client_fd, (struct sockaddr*)(&peeraddr), &peeraddrlen);

							print(&localaddr);
							print(&peeraddr);
							

							ev.data.fd = client_fd;
							ev.events = EPOLLIN;
							epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
						}
					}
					else {
						std::cout << "can send data" << std::endl;
					}
				}
				if (events[i].events && (EPOLLERR))
				{
					int optval;
					socklen_t optlen = static_cast<socklen_t>(sizeof optval);
					//获取一个套接字的各种数据
					int error = getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);

					//EINPROGRESS  Operation now in progress
					std::cout << " errno:" << errno << "  socket is error? " << (error < 0 ? "true" : "false") << std::endl;
				}


			

		}
	}
}
int main(int argc, char*argv[])
{
	if (argc < 3)
	{
		std::cout << "ip port <0|1>" << std::endl;
		return 0;
	}
	int i = atoi(argv[3]);
	if (i == 0)
	{
		server(argv);
	}
	else {
		client(argv);
	}
	
	


}