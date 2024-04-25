#ifndef CCLIENTSOCKET_HPP
#define CCLIENTSOCKET_HPP

#include <iostream>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <sys/poll.h>
#include <unistd.h>
#include "ServerSocket.hpp"

class CClientSocket : ServerSocket{
	private:
		std::vector<int> 	_connectedClientSockets;
		std::vector<pollfd>	pfds;

	public:
		2ClientSocket();
		~2ClientSocket();

		void	handleInputEvent(int index, int server_fd);
		void	handleIncomingConnection(int server_fd);
		void	addClientSocketToFds(int client_fd);
		void	startPolling(int server_fd);
};

#endif