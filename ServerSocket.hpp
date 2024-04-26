#ifndef ServerSocket_HPP
#define ServerSocket_HPP

#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h> //to convert ip into string
#include <netdb.h>

#define	MYPORT 8080
#define	BACKLOG 10

class ServerSocket{
	private:
		struct sockaddr_in	_server_addr;

	public:
		ServerSocket();
		~ServerSocket();

		int					createServerSocket();
		struct sockaddr_in	defineServerAddress();
		void				bindServerSocket(int server_fd, struct sockaddr_in &server_addr);
		void				listenIncomingConnections(int server_fd);
		int					setUpServerSocket();
};

#endif