#include "Webserv.hpp"
#include <memory>

#define CLOSE 1

ClientSocket::ClientSocket() {}

ClientSocket::~ClientSocket() {
  for (size_t i = 0; i < _connectedClientSockets.size(); ++i) {
    int client_fd = _connectedClientSockets[i];
    close(client_fd);
  }
}

int ClientSocket::checkExitSignals(char *buffer, int client_fd) {
  std::string bufferstr = buffer;
  if (bufferstr.find("close") != std::string::npos) {
    std::cout << "Received close command from client. Closing connection."
              << std::endl;
    close(client_fd);
    return (0);
  }
  if (bufferstr.find("exit") != std::string::npos) {
    std::cout
        << "Received exit command from client. Closing connection and program."
        << std::endl;
    close(client_fd);
    return (CLOSE);
  }
  return (0);
}

// int ClientSocket::handleInputEvent(int index) {
// 	char buffer[1024];
// 	int client_fd = _polledfds[index].fd;

// 	const char* messageFromServer = "Server received data \n";
// 	ssize_t bytesSent = send(client_fd, messageFromServer,
// strlen(messageFromServer), 0); 	if (bytesSent == -1) {
// std::cerr << "Error sending data to client: " << strerror(errno) <<
// std::endl; 		return (0);
// 	}
// 	ssize_t bytesRead = recv(client_fd, buffer, sizeof(buffer), 0);
// 	if (bytesRead == -1) {
// 		std::cerr << "Error receiving data from client: " <<
// strerror(errno) << std::endl; 		return (0); 	} else if
// (bytesRead == 0) { 		std::cout << "Client disconnected" << std::endl;
// return (0);
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
    std::cerr << "Error receiving data from client: " << strerror(errno)
              << std::endl;
    return (0);
  } else if (bytesRead == 0) {
    std::cout << "Client disconnected" << std::endl;
    _polledfds[index].revents = POLLERR;
    return (0);
  }
  buffer[bytesRead] = '\0';

  if (checkExitSignals(buffer, client_fd) == CLOSE)
    return (CLOSE);

  auto request = std::make_shared<Request>(buffer);
  request->printRequest();
  Response response(request);

  const std::string messageFromServer = response.get_response();
  ssize_t bytesSent =
      send(client_fd, messageFromServer.c_str(), messageFromServer.length(), 0);
  if (bytesSent == -1) {
    std::cerr << "Error sending data to client: " << strerror(errno)
              << std::endl;
    return (0);
  }

  _polledfds[index].revents = POLLOUT;
  return (0);
}

void ClientSocket::addClientSocketToFds(int serverSocket_fd) {
  // Add server socket
  pollfd server_pollfd;
  server_pollfd.fd = serverSocket_fd;
  server_pollfd.events = POLLIN;
  _polledfds.push_back(server_pollfd);

  // Add client sockets
  for (size_t i = 0; i < _connectedClientSockets.size(); ++i) {
    pollfd client_pollfd;
    client_pollfd.fd = _connectedClientSockets[i];
    client_pollfd.events = POLLIN;
    _polledfds.push_back(client_pollfd);
  }
}

void ClientSocket::acceptClient(int serverSocket_fd) {
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  int client_fd =
      accept(serverSocket_fd, (struct sockaddr *)&clientAddr, &clientAddrLen);
  if (client_fd == -1) {
    std::cerr << "Error accepting connection" << std::endl;
    return;
  }
  std::cout << "Accepted new connection on server socket " << serverSocket_fd
            << " from client socket " << client_fd << std::endl;
  _connectedClientSockets.push_back(client_fd);
}

void ClientSocket::removeClientSocket(int client_fd) {
  close(client_fd);

  auto it = std::find(_connectedClientSockets.begin(),
                      _connectedClientSockets.end(), client_fd);
  if (it != _connectedClientSockets.end()) {
    _connectedClientSockets.erase(it);
  }

  for (auto it = _polledfds.begin(); it != _polledfds.end(); it++) {
    if (it->fd == client_fd) {
      _polledfds.erase(it);
      break;
    }
  }
  std::cout << "Client socket " << client_fd << " removed" << std::endl;
}

void ClientSocket::startPolling(int serverSocket_fd) {
  while (true) {
    _polledfds.clear(); // Clear the vector before each iteration
    // for (int i = 0; i < _connectedClientSockets.size(); i++) {
    // 	std::cout << "added client socked fd" << _connectedClientSockets[i] <<
    // std::endl; 	addClientSocketToFds(_connectedClientSockets[i]);
    // }
    addClientSocketToFds(serverSocket_fd);
    int poll_count = poll(_polledfds.data(), _polledfds.size(), 15000);
    if (poll_count > 0) {
      for (size_t i = 0; i < _polledfds.size(); i++) {
        if (_polledfds[i].revents & POLLIN) {
          if (_polledfds[i].fd == serverSocket_fd) {
            acceptClient(serverSocket_fd);
          } else {
            std::cout << "Input available on descriptor " << _polledfds[i].fd
                      << std::endl;
            if (handleInputEvent(i) == CLOSE)
              return;
            removeClientSocket(_polledfds[i].fd);
            i--;
          }
        }
        if (_polledfds[i].revents & POLLOUT) {
          std::cout << "Socket " << _polledfds[i].fd << " is ready for writing"
                    << std::endl;
        }
        if (_polledfds[i].revents & (POLLHUP | POLLERR)) {
          std::cerr << "Error or disconnection on descriptor "
                    << _polledfds[i].fd << std::endl;
          removeClientSocket(_polledfds[i].fd);
          i--;
        }
      }
    } else if (poll_count == 0) {
      std::cout << "Poll timeout" << std::endl;

    } else {
      std::cerr << "Poll error: " << strerror(errno) << std::endl;
    }
  }
}
