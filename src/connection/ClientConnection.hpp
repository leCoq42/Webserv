#pragma once

#include <iostream>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ServerConnection.hpp"
#include "../log/log.hpp"
#include <iterator>
#include <algorithm> 
#include <memory>
#include <netinet/tcp.h>

struct ClientInfo {
	unsigned long	clientID;
	int 			clientFD;
	char 			clientIP[INET_ADDRSTRLEN];
	bool 			keepAlive;
	long int		timeOut;
	long int		lastRequestTime;
	size_t 			numRequests;
	size_t 			maxRequests;
};

class ClientSocket : ServerSocket, Log {
	private:
			std::shared_ptr<ServerSocket> 	ptrServerSocket;
			std::vector<ClientInfo> 		_connectedClients;
			std::vector<pollfd>				_pollfdContainer;

			public:
				ClientSocket();
				ClientSocket(std::shared_ptr<ServerSocket> serverSocket);
				~ClientSocket();

				void		handleInputEvent(int index);
				void		acceptClients(int server_fd);
				void		addSocketsToPollfdContainer();
				void		startPolling();
				void		removeClientSocket(int client_fd);
				bool 		isServerSocket(int fd);
				void 		handlePollOutEvent(size_t index);
				void		handlePollErrorEvent(size_t index);
				ClientInfo	initClientInfo(int client_fd, sockaddr_in clientAddr);
				void		manageKeepAlive(int index);
				void		checkConnectedClientsStatus();
				int			getIndexByClientFD(int client_fd);
};
