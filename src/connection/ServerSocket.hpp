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
#include <vector>

//
#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>     /* atoi */
#include "../src/parser_0/inc/parser.hpp" //

#define	BACKLOG 10

class ServerSocket{
	private:
		struct sockaddr_in	_server_addr;
	
	protected:

	public:
		std::vector<int>	_vecServerSockets;
		ServerSocket();
		~ServerSocket();

		int					createServerSocket();
		struct sockaddr_in	defineServerAddress(std::list<std::string>::iterator it);
		void				bindServerSocket(int server_fd, struct sockaddr_in &server_addr);
		void				listenIncomingConnections(int server_fd);
		void				setUpServerSockets(ServerStruct serverinfo);
};

#endif