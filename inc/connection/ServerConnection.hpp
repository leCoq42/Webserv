#pragma once

#include <arpa/inet.h> 
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include "ServerStruct.hpp"
#include "log.hpp"
#include <signal.h>

#define BACKLOG 512
#define success 0
#define failed -1

struct ServerInfo {
	std::string         serverID;
	int                 serverFD;
	int                 serverPort;
	size_t			    MaxBodySize;
	struct sockaddr_in  server_addr;
};

class ServerConnection {
	private:
		std::shared_ptr<Log> _log;

	public:
		std::vector<ServerInfo> _connectedPorts;
		ServerConnection(std::shared_ptr<Log> log);
		~ServerConnection();

		void	initSocketInfo(ServerStruct &serverStruct, ServerInfo &info,
								std::list<std::string>::iterator it);
		int 	createSocket(ServerInfo &info);
		int 	bindPort(ServerInfo &info);
		int 	listenOnPort(ServerInfo &info);
		int 	setUpPorts(ServerStruct &serverStruct);
};
