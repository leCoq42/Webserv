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

class ClientSocket : ServerSocket{
	private:
		std::vector<int> 	_connectedClientSockets;
		std::vector<pollfd>	_polledfds;

	public:
		ClientSocket();
		~ClientSocket();

		int		handleInputEvent(int index);
		void	acceptClient(int serverSocket_fd);
		void	addClientSocketToFds(int client_fd);
		void	startPolling(int server_fd);
		int		checkExitSignals(char *buffer, int client_fd);
		void	removeClientSocket(int client_fd);
};

#endif