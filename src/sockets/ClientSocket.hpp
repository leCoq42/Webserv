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

class ClientSocket : ServerSocket{
	private:
			std::vector<int> 	_connectedClientSockets;
			std::vector<pollfd>	_polledfds;
			std::shared_ptr<ServerSocket> ptrServerSocket;

			public:
				ClientSocket();
				ClientSocket(std::shared_ptr<ServerSocket> serverSocket);
				~ClientSocket();

				int		handleInputEvent(int index);
				void acceptClients(int server_fd);
				void	addClientToServer_fd();
				void	startPolling();
				int		checkExitSignals(char *buffer, int client_fd);
				void	removeClientSocket(int client_fd);

				bool 	isServerSocket(int fd);
};

#endif