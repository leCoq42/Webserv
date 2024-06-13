#include "ClientConnection.hpp"
#include "../request/Request.hpp"

ClientConnection::ClientConnection() {
}

ClientConnection::ClientConnection(std::shared_ptr<ServerConnection> ServerConnection) : ptrServerConnection(ServerConnection){
}

ClientConnection::~ClientConnection() {
	for (size_t i = 0; i < _connectedClients.size(); ++i) {
		logClientConnection("connection closed", _connectedClients[i].clientIP, _connectedClients[i].clientFD);
		close(_connectedClients[i].clientFD);
	}
}

int	ClientConnection::getIndexByClientFD(int clientFD) {
	int index = 0;
	for (size_t i = 0; i < _connectedClients.size(); i++) {
		if (_connectedClients[i].clientFD == clientFD)
			index = i;
	}
	return (index);
}

void	ClientConnection::manageKeepAlive(int index) {
	int indexConnectedClients = getIndexByClientFD(_pollfdContainer[index].fd);
	time_t currentTime;
	time(&currentTime);
	_connectedClients[indexConnectedClients].lastRequestTime = currentTime;
	_connectedClients[indexConnectedClients].numRequests += 1;
	if (_connectedClients[indexConnectedClients].keepAlive == true)
		_pollfdContainer[index].events = POLLIN | POLLOUT;
	else
		removeClientSocket(_pollfdContainer[index].fd);
}

void	ClientConnection::handleInputEvent(int index) {
	char buffer[1024];

	ssize_t bytesRead = recv(_pollfdContainer[index].fd, buffer, sizeof(buffer), 0);
	if (bytesRead == -1) {
		logClientError("Failed to receive data from client", _connectedClients[getIndexByClientFD(_pollfdContainer[index].fd)].clientIP, _pollfdContainer[index].fd);
		return;
	} 
	else if (bytesRead == 0) {
		logClientError("Client disconnected", _connectedClients[getIndexByClientFD(_pollfdContainer[index].fd)].clientIP, _pollfdContainer[index].fd);
		_pollfdContainer[index].revents = POLLERR;
		return;
	}
	buffer[bytesRead] = '\0';
	
	Request request(buffer);
	request.parseRequest();
	const char* httpResponse = "here is the response from the server\n";
	ssize_t bytesSent = send(_pollfdContainer[index].fd, httpResponse, strlen(httpResponse), 0);
	if (bytesSent == -1) {
		std::cerr << "Failed to send data to client: " << strerror(errno) << std::endl;
		return;
	} 
	else if (bytesSent == 0) {
		logClientError("Client disconnected", _connectedClients[getIndexByClientFD(_pollfdContainer[index].fd)].clientIP, _pollfdContainer[index].fd);
		_pollfdContainer[index].revents = POLLERR;
	}
	manageKeepAlive(index);
}

void	ClientConnection::addSocketsToPollfdContainer() {
	for (size_t i = 0; i < ptrServerConnection->_connectedServers.size(); i++) {
		pollfd server_pollfd;
		server_pollfd.fd = ptrServerConnection->_connectedServers[i].serverFD;
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

ClientInfo	ClientConnection::initClientInfo(int clientFD, sockaddr_in clientAddr) {
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
	clientInfo.maxRequests = 3; // will be configurable later
	return (clientInfo);
}

void	ClientConnection::acceptClients(int serverFD) {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientFD = accept(serverFD, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (clientFD == -1) {
		logError("Failed to connect on server socket");
		return;
	}
	if (getpeername(clientFD, (struct sockaddr*)&clientAddr, &clientAddrLen) != 0)
		logError("Failed to read client IP");
	_connectedClients.push_back(initClientInfo(clientFD, clientAddr));
	logClientConnection("accepted connection", _connectedClients.back().clientIP, clientFD);
}

void	ClientConnection::removeClientSocket(int clientFD) {
	close(clientFD);
	logClientConnection("closed connection", _connectedClients[getIndexByClientFD(clientFD)].clientIP, clientFD);
	int indexConnectedClients = getIndexByClientFD(clientFD);
	_connectedClients.erase(indexConnectedClients + _connectedClients.begin());
	for (auto it = _pollfdContainer.begin(); it != _pollfdContainer.end(); ++it) {
		if (it->fd == clientFD) {
			_pollfdContainer.erase(it);
			break;
		}
	}
}

bool	ClientConnection::isServerSocket(int fd) {
	for (const auto& server_fd : ptrServerConnection->_connectedServers) {
		if (fd == server_fd.serverFD)
			return true;
	}
	return (false);
}

void	ClientConnection::handlePollOutEvent(size_t index) {
	_pollfdContainer[index].events &= ~POLLOUT;
}

void	ClientConnection::handlePollErrorEvent(size_t index) {
	logClientError("Disconnection on descriptor", _connectedClients[getIndexByClientFD(_pollfdContainer[index].fd)].clientIP, _pollfdContainer[index].fd);
	removeClientSocket(_pollfdContainer[index].fd);
}

void	ClientConnection::checkConnectedClientsStatus() {
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

void	ClientConnection::setUpClientConnection() {
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
					else 
						handleInputEvent(i);
				}
				if (_pollfdContainer[i].revents & POLLOUT)
					handlePollOutEvent(i);
				if (_pollfdContainer[i].revents & (POLLHUP | POLLERR)) {
					handlePollErrorEvent(i);
					i--;
				}
			}
		} 
		else if (poll_count == 0)
			continue;
		else 
			logError("Failed to poll");
	}
}