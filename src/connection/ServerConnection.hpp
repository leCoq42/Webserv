#pragma once

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

struct ServerInfo{
	std::string 		serverID;
	int					serverFD;
	int 				serverPort;
	struct sockaddr_in 	server_addr;
};

class ServerSocket{
	private:

	public:
		std::vector<ServerInfo>	_connectedServers;
		ServerSocket();
		~ServerSocket();

		void				initServerInfo(ServerStruct serverStruct, ServerInfo& info, std::list<std::string>::iterator it);
		void				createServerSocket(ServerInfo& info);
		void				bindServerSocket(ServerInfo& info);
		void				listenIncomingConnections(ServerInfo &info);
		void				setUpServerSockets(ServerStruct serverStruct);
};