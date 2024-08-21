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

#define BACKLOG 512

struct ServerInfo {
	std::string         serverID;
	int                 serverFD;
	int                 serverPort;
	struct sockaddr_in  server_addr;
};

class ServerConnection {
	private:
		std::shared_ptr<Log> _log;

	public:
		std::vector<ServerInfo> _connectedServers;
		ServerConnection(std::shared_ptr<Log> log);
		~ServerConnection();

		void					initServerInfo(ServerStruct &serverStruct, ServerInfo &info,
												std::list<std::string>::iterator it);
		void 					createServerSocket(ServerInfo &info);
		void 					bindServerSocket(ServerInfo &info);
		void 					listenIncomingConnections(ServerInfo &info);
		void 					setUpServerConnection(ServerStruct &serverStruct);
};
