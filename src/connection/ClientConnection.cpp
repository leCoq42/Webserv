#include "ClientConnection.hpp"
#include "../request/Request.hpp"

ClientSocket::ClientSocket() {
}

ClientSocket::ClientSocket(std::shared_ptr<ServerSocket> ServerSocket) : ptrServerSocket(ServerSocket){
}

ClientSocket::~ClientSocket() {
	for (size_t i = 0; i < _connectedClients.size(); ++i) {
		close(_connectedClients[i].clientFD);
	}
}

int	ClientSocket::getIndexByClientFD(int clientFD) {
	for (size_t i = 0; i < _connectedClients.size(); i++) {
		if (_connectedClients[i].clientFD == clientFD) {
			return i;
		}
	}
	return -1;
}

void	ClientSocket::manageKeepAlive(int index) {
	int clientFD = _pollfdContainer[index].fd;
	auto it = std::find_if(_connectedClients.begin(), _connectedClients.end(),
						   [clientFD](const ClientInfo& client) {
							   return client.clientFD == clientFD;
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
	// logAccess(_connectedClients[getIndexByClientFD(_pollfdContainer[index].fd)].clientIP, request. , 200, bytesSent, "localhost", "Mozilla/5.0");
	manageKeepAlive(index);
}

void	ClientSocket::addSocketsToPollfdContainer() {
	for (size_t i = 0; i < ptrServerSocket->_connectedServers.size(); i++) {
		pollfd server_pollfd;
		server_pollfd.fd = ptrServerSocket->_connectedServers[i].serverFD;
		server_pollfd.events = POLLIN;
		_pollfdContainer.push_back(server_pollfd);
	}

	for (size_t i = 0; i < _connectedClients.size(); ++i) {
		pollfd client_pollfd;
		client_pollfd.fd = _connectedClients[i].clientFD;
		client_pollfd.events = POLLIN;
		_pollfdContainer.push_back(client_pollfd);
	}
}

ClientInfo	ClientSocket::initClientInfo(int clientFD, sockaddr_in clientAddr) {
	ClientInfo clientInfo;
	memset(&clientInfo, 0, sizeof(clientInfo));
	time_t currentTime;
	time(&currentTime);
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientInfo.clientIP, sizeof(clientInfo.clientIP));
	clientInfo.clientFD = clientFD;
	clientInfo.keepAlive = true;
	clientInfo.timeOut = 10; // will be configurable later
	clientInfo.lastRequestTime = currentTime;
	clientInfo.numRequests = 0;
	clientInfo.maxRequests = 10; // will be configurable later
	return (clientInfo);
}

void	ClientSocket::acceptClients(int serverFD) {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int client = accept(serverFD, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (client == -1) {
		std::cerr << "Error accepting connection on server socket " << serverFD << ": " << strerror(errno) << std::endl;
		return;
	}
	if (getpeername(client, (struct sockaddr*)&clientAddr, &clientAddrLen) != 0) {
		#ifdef DEBUG
			std::cerr << "Failed to read client IP" << std::endl;
			return;
		#endif
	}
	_connectedClients.push_back(initClientInfo(client, clientAddr));
	#ifdef DEBUG
		std::cout << "Accepted new connection on server socket " << serverFD << " from client " << clientFD << std::endl;
	#endif
}

void	ClientSocket::removeClientSocket(int clientFD) {
	close(clientFD);
	auto it = std::find_if(_connectedClients.begin(), _connectedClients.end(), 
						   [clientFD](const ClientInfo& clientInfo) { 
							   return clientInfo.clientFD == clientFD; 
						   });
	if (it != _connectedClients.end()) {
		_connectedClients.erase(it);
	}

	for (auto it = _pollfdContainer.begin(); it != _pollfdContainer.end(); ++it) {
		if (it->fd == clientFD) {
			_pollfdContainer.erase(it);
			break;
		}
	}
	#ifdef DEBUG
		std::cout << "Client socket " << clientFD << " removed" << std::endl;
	#endif
}

bool	ClientSocket::isServerSocket(int fd) {
	for (const auto& server_fd : ptrServerSocket->_connectedServers) {
		if (fd == server_fd.serverFD) {
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
				removeClientSocket(_connectedClients[i].clientFD);
		if (_connectedClients[i].numRequests >= _connectedClients[i].maxRequests 
			&& _connectedClients[i].keepAlive == true)
				removeClientSocket(_connectedClients[i].clientFD);
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
							std::cout << "Input available on clientFD " << _pollfdContainer[i].fd << std::endl;
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