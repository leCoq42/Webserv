#include "ClientSocket.hpp"
#include "../request/Request.hpp"

ClientSocket::ClientSocket() {
}

ClientSocket::ClientSocket(std::shared_ptr<ServerSocket> ServerSocket) : ptrServerSocket(ServerSocket){
}

ClientSocket::~ClientSocket() {
	for (size_t i = 0; i < _connectedClients.size(); ++i) {
		int client_fd = _connectedClients[i].clientFd;
		close(client_fd);
	}
}

void	ClientSocket::manageKeepAlive(int index) {
	int client_fd = _pollfdContainer[index].fd;
	auto it = std::find_if(_connectedClients.begin(), _connectedClients.end(),
						   [client_fd](const ClientInfo& client) {
							   return client.clientFd == client_fd;
						   });
	if (it != _connectedClients.end()) {
		time_t currentTime;
		time(&currentTime);
		it->lastRequestTime = currentTime;
		it->numRequests += 1;
	}
	if (it->keepAlive == true) {
		_pollfdContainer[index].events = POLLIN | POLLOUT;
	} 
	else {
		removeClientSocket(_pollfdContainer[index].fd);
	}
}

void	ClientSocket::handleInputEvent(int index) {
	char buffer[1024];

	ssize_t bytesRead = recv(_pollfdContainer[index].fd, buffer, sizeof(buffer), 0);
	if (bytesRead == -1) {
		std::cerr << "Error receiving data from client: " << strerror(errno) << std::endl;
		return;
	} else if (bytesRead == 0) {
		#ifdef DEBUG
			std::cout << "Client disconnected" << std::endl;
		#endif
		_pollfdContainer[index].revents = POLLERR;
		return;
	}
	buffer[bytesRead] = '\0';
	
	Request request(buffer);
	request.parseRequest();

	int valid = request.checkRequestValidity();
	const char* httpResponse = "here is the response from the server";
	ssize_t bytesSent = send(_pollfdContainer[index].fd, httpResponse, strlen(httpResponse), 0);
	if (bytesSent == -1) {
		std::cerr << "Error sending data to client: " << strerror(errno) << std::endl;
		return;
	} 
	else if (bytesSent == 0) {
		#ifdef DEBUG
		std::cout << "Client disconnected" << std::endl;
		#endif
		_pollfdContainer[index].revents = POLLERR;
	}
	manageKeepAlive(index);
}

void	ClientSocket::addSocketsToPollfdContainer() {
	for (size_t i = 0; i < ptrServerSocket->_vecServerSockets.size(); i++) {
		pollfd server_pollfd;
		server_pollfd.fd = ptrServerSocket->_vecServerSockets[i];
		server_pollfd.events = POLLIN;
		_pollfdContainer.push_back(server_pollfd);
	}

	for (size_t i = 0; i < _connectedClients.size(); ++i) {
		pollfd client_pollfd;
		client_pollfd.fd = _connectedClients[i].clientFd;
		client_pollfd.events = POLLIN;
		_pollfdContainer.push_back(client_pollfd);
	}
}

ClientInfo	ClientSocket::initClientInfo(int client_fd, sockaddr_in clientAddr) {
	ClientInfo clientInfo;
	time_t currentTime;
	time(&currentTime);
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientInfo.clientIP, sizeof(clientInfo.clientIP));
	clientInfo.clientFD = client_fd;
	clientInfo.keepAlive = true;
	clientInfo.timeOut = 3; // will be configurable later
	clientInfo.lastRequestTime = currentTime;
	clientInfo.numRequests = 0;
	clientInfo.maxRequests = 3; // will be configurable later
	return (clientInfo);
}

void	ClientSocket::acceptClients(int server_fd) {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int client_fd = accept(server_fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (client_fd == -1) {
		std::cerr << "Error accepting connection on server socket " << server_fd << ": " << strerror(errno) << std::endl;
		return;
	}
	if (getpeername(client_fd, (struct sockaddr*)&clientAddr, &clientAddrLen) != 0) {
		#ifdef DEBUG
			std::cerr << "Failed to read client IP" << std::endl;
			return;
		#endif
	}
	_connectedClients.push_back(initClientInfo(client_fd, clientAddr));
	#ifdef DEBUG
		std::cout << "Accepted new connection on server socket " << server_fd << " from client_fd " << client_fd << std::endl;
	#endif
}

void	ClientSocket::removeClientSocket(int client_fd) {
	close(client_fd);

	auto it = std::find_if(_connectedClients.begin(), _connectedClients.end(), 
						   [client_fd](const ClientInfo& clientInfo) { 
							   return clientInfo.clientFD == client_fd; 
						   });
	if (it != _connectedClients.end()) {
		_connectedClients.erase(it);
	}

	for (auto it = _pollfdContainer.begin(); it != _pollfdContainer.end(); ++it) {
		if (it->fd == client_fd) {
			_pollfdContainer.erase(it);
			break;
		}
	}
	#ifdef DEBUG
		std::cout << "Client socket " << client_fd << " removed" << std::endl;
	#endif
}

bool	ClientSocket::isServerSocket(int fd) {
	for (const auto& server_fd : ptrServerSocket->_vecServerSockets) {
		if (fd == server_fd) {
			return true;
		}
	}
	return (false);
}

void	ClientSocket::handlePollOutEvent(size_t index) {
	#ifdef DEBUG
	std::cout << "Socket " << _pollfdContainer[index].fd << " is ready for writing" << std::endl;
	#endif
	_pollfdContainer[index].events &= ~POLLOUT;
}

void	ClientSocket::handlePollErrorEvent(size_t index) {
	#ifdef DEBUG
	std::cerr << "Error or disconnection on descriptor " << _pollfdContainer[index].fd << std::endl;
	#endif
	removeClientSocket(_pollfdContainer[index].fd);
}

void	ClientSocket::checkConnectedClientsStatus() {
	time_t currentTime;
	time(&currentTime);
	for (size_t i = 0; i < _connectedClients.size(); i++) {
		if (currentTime - _connectedClients[i].lastRequestTime > _connectedClients[i].timeOut 
			&& _connectedClients[i].keepAlive == true) 
				removeClientSocket(_connectedClients[i].clientFd);
		if (_connectedClients[i].numRequests >= _connectedClients[i].maxRequests 
			&& _connectedClients[i].keepAlive == true)
				removeClientSocket(_connectedClients[i].clientFd);
	}
}

void	ClientSocket::startPolling() {
	try{
		while (true) {
			_pollfdContainer.clear();
			addSocketsToPollfdContainer();
			int poll_count = poll(_pollfdContainer.data(), _pollfdContainer.size(), 100);
			checkConnectedClientsStatus();
			if (poll_count > 0) {
				for (size_t i = 0; i < _pollfdContainer.size(); i++) {
					if (_pollfdContainer[i].revents & POLLIN) {
						if (isServerSocket(_pollfdContainer[i].fd)) 
							acceptClients(_pollfdContainer[i].fd);
						else {
							#ifdef DEBUG
							std::cout << "Input available on client_fd " << _pollfdContainer[i].fd << std::endl;
							#endif
							handleInputEvent(i);
						}
					}
					if (_pollfdContainer[i].revents & POLLOUT) {
						handlePollOutEvent(i);
					}
					if (_pollfdContainer[i].revents & (POLLHUP | POLLERR)) {
						handlePollErrorEvent(i);
						i--;
					}
				}
			} 
			else if (poll_count == 0) {
				continue;
			}
			else {
				throw std::runtime_error(strerror(errno));
			}
		}
	} 
	catch (const std::exception &e) {
		std::cerr << "Error polling: " << e.what() << std::endl;
	}
}