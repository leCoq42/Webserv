#pragma once

#include <iostream>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ServerConnection.hpp"
#include <iterator>
#include <algorithm> 
#include <memory>
#include <netinet/tcp.h>

struct ClientInfo {
	unsigned long	clientID;           // Client ID
	int 			clientFD;           // Client file descriptor
	char 			clientIP[INET_ADDRSTRLEN];
	bool 			keepAlive;			// Keep-alive status
	size_t 			timeOut;			// Timeout value
	time_t 			lastRequestTime; 	// Timestamp of the last request
	size_t 			numRequests;        // Number of requests made so far
	size_t 			maxRequests;        // Maximum number of requests allowed
};

class ClientSocket : ServerSocket {
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
};
