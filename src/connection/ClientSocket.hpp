#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include <iostream>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ServerSocket.hpp"
#include <iterator>
#include <algorithm> 
#include <memory>
#include <netinet/tcp.h>

struct ClientInfo {
    int 	clientFd;           // Client file descriptor
	bool 	keepAlive;			// Keep-alive status
	size_t 	timeOut;			// Timeout value
    time_t 	lastRequestTime; 	// Timestamp of the last request
    size_t 	numRequests;        // Number of requests made so far
    size_t 	maxRequests;        // Maximum number of requests allowed
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
				void		manageKeepAlive(int index);
				void		checkConnectedClientsStatus();
};

#endif