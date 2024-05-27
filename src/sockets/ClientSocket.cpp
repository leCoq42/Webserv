#include "ClientSocket.hpp"
#include "../request/Request.hpp"

#define	CLOSE	1

ClientSocket::ClientSocket() {
}

ClientSocket::ClientSocket(std::shared_ptr<ServerSocket> ServerSocket) : ptrServerSocket(ServerSocket){
}

ClientSocket::~ClientSocket() {
	for (size_t i = 0; i < _connectedClientSockets.size(); ++i) {
		int client_fd = _connectedClientSockets[i];
		close(client_fd);
	}
}

int	ClientSocket::checkExitSignals(char *buffer, int client_fd) {
	std::string bufferstr = buffer;
	if (bufferstr.find("close") != std::string::npos) {
		std::cout << "Received close command from client. Closing connection." << std::endl;
		close(client_fd);
		return (0);
	}
	if (bufferstr.find("exit") != std::string::npos) {
		std::cout << "Received exit command from client. Closing connection and program." << std::endl;
		close(client_fd);
		return (CLOSE);
	}
	return (0);
}

// int ClientSocket::handleInputEvent(int index) {
// 	char buffer[1024];
// 	int client_fd = _polledfds[index].fd;

// 	const char* messageFromServer = "Server received data \n";
// 	ssize_t bytesSent = send(client_fd, messageFromServer, strlen(messageFromServer), 0);
// 	if (bytesSent == -1) {
// 		std::cerr << "Error sending data to client: " << strerror(errno) << std::endl;
// 		return (0);
// 	}
// 	ssize_t bytesRead = recv(client_fd, buffer, sizeof(buffer), 0);
// 	if (bytesRead == -1) {
// 		std::cerr << "Error receiving data from client: " << strerror(errno) << std::endl;
// 		return (0);
// 	} else if (bytesRead == 0) {
// 		std::cout << "Client disconnected" << std::endl;
// 		return (0);
// 	}
// 	buffer[bytesRead] = '\0';
// 	std::cout << "Received data from client: " << buffer << std::endl;
// 	if (checkExitSignals(buffer, client_fd) == CLOSE)
// 		return (CLOSE);
// 	_polledfds[index].revents = POLLOUT;
// 	return (0);
// }

int ClientSocket::handleInputEvent(int index) {
	char buffer[1024];
	int client_fd = _polledfds[index].fd;

	ssize_t bytesRead = recv(client_fd, buffer, sizeof(buffer), 0);
	if (bytesRead == -1) {
		std::cerr << "Error receiving data from client: " << strerror(errno) << std::endl;
		return (0);
	} else if (bytesRead == 0) {
		std::cout << "Client disconnected" << std::endl;
		_polledfds[index].revents = POLLERR;
		return (0);
	}
	buffer[bytesRead] = '\0';

	// Testing purposses
	if (checkExitSignals(buffer, client_fd) == CLOSE)
		return (CLOSE);
	
	Request request(buffer);
	request.parseRequest();

	int valid = request.checkRequestValidity();
	if (valid == 1)
		std::cout << "request is valid" << std::endl;
	else
		std::cout << "request is not valid" << std::endl;


	const char* httpResponse = 
	    "HTTP/1.1 200 OK\r\n"
	    "Content-Type: text/plain\r\n"
	    "Content-Length: 23\r\n"
	    "\r\n"
	    "Server received data\n";
	ssize_t bytesSent = send(client_fd, httpResponse, strlen(httpResponse), 0);
	if (bytesSent == -1) {
	    std::cerr << "Error sending data to client: " << strerror(errno) << std::endl;
	    return (0);
	}
	  _polledfds[index].events = POLLOUT;
	return (0);
}

void ClientSocket::addClientToServer_fd() {
	// Add server socket
	for (size_t i = 0; i < ptrServerSocket->_vecServerSockets.size(); i++) {
		pollfd server_pollfd;
		server_pollfd.fd = ptrServerSocket->_vecServerSockets[i];
		server_pollfd.events = POLLIN;
		_polledfds.push_back(server_pollfd);
		std::cout << "server_fd: " << ptrServerSocket->_vecServerSockets[i] << std::endl;
	}

	// Add client sockets
	for (size_t i = 0; i < _connectedClientSockets.size(); ++i) {
		pollfd client_pollfd;
		client_pollfd.fd = _connectedClientSockets[i];
		client_pollfd.events = POLLIN;
		_polledfds.push_back(client_pollfd);
		std::cout << "client_fd: " << _connectedClientSockets[i] << std::endl;
	}
}

void ClientSocket::acceptClients(int server_fd) {
	if (!ptrServerSocket) {
		std::cerr << "Error: Server socket pointer is null." << std::endl;
		return;
	}

	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);

	std::cout << "ServerSockets  " << server_fd << std::endl;
	int client_fd = accept(server_fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
	std::cout << "Client_fd: " << client_fd << std::endl;
	if (client_fd == -1) {
		std::cerr << "Error accepting connection on server socket " 
                     << server_fd << ": " 
                     << strerror(errno) << std::endl;
	}
	std::cout << "Accepted new connection on server socket " 
                 << server_fd << " from client socket " << client_fd << std::endl;
	_connectedClientSockets.push_back(client_fd);
}

void ClientSocket::removeClientSocket(int client_fd) {
	close(client_fd);

	auto it = std::find(_connectedClientSockets.begin(), _connectedClientSockets.end(), client_fd);
	if (it != _connectedClientSockets.end()) {
		_connectedClientSockets.erase(it);
	}

	for (auto it = _polledfds.begin(); it != _polledfds.end(); ++it) {
		if (it->fd == client_fd) {
			_polledfds.erase(it);
			break;
		}
	}
	std::cout << "Client socket " << client_fd << " removed" << std::endl;
}


bool ClientSocket::isServerSocket(int fd) {
    for (const auto& server_fd : ptrServerSocket->_vecServerSockets) {
        if (fd == server_fd) {
            return true;
        }
    }
    return false;
}

void ClientSocket::startPolling() {
    while (true) {
        _polledfds.clear();
        addClientToServer_fd();
        int poll_count = poll(_polledfds.data(), _polledfds.size(), 20000);
        if (poll_count > 0) {
            for (size_t i = 0; i < _polledfds.size(); i++) {
                if (_polledfds[i].revents & POLLIN) {
                    if (isServerSocket(_polledfds[i].fd)) 
                        acceptClients(_polledfds[i].fd);
					else {
						std::cout << "Input available on descriptor " << _polledfds[i].fd << std::endl;
                        handleInputEvent(i);
					}
                }
                if (_polledfds[i].revents & POLLOUT) {
                    std::cout << "Socket " << _polledfds[i].fd << " is ready for writing" << std::endl;
                    _polledfds[i].events &= ~POLLOUT; // Disable POLLOUT after handling
                }
                if (_polledfds[i].revents & (POLLHUP | POLLERR)) {
                    std::cerr << "Error or disconnection on descriptor " << _polledfds[i].fd << std::endl;
                    removeClientSocket(_polledfds[i].fd);
                    i--;
                }
            }
        } 
		else if (poll_count == 0) {
            std::cout << "Poll timeout" << std::endl;
        }
		else {
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
        }
    }
}
