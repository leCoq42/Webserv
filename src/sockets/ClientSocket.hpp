#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include <iostream>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <sys/poll.h>
#include <unistd.h>
#include "ServerSocket.hpp"
#include <iterator>
#include <algorithm> 
#include <memory>

struct ClientInfo {
	bool 	keepAlive;			// Keep-alive status
    int 	clientFd;           // Client file descriptor
    time_t 	lastRequestTime; 	// Timestamp of the last request
    int 	numRequests;        // Number of requests made so far
    int 	maxRequests;        // Maximum number of requests allowed
};

class ClientSocket : ServerSocket{
	private:
			std::vector<ClientInfo> 		_connectedClients;
			std::vector<pollfd>				_pollfdContainer;
			std::shared_ptr<ServerSocket> 	ptrServerSocket;

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
				ClientInfo	initClientInfo(int client_fd);
				void		manageClientConnection(int index);
				void		checkConnectedClientsStatus();
};

#endif