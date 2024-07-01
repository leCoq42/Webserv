#include "ClientConnection.hpp"
#include "request.hpp"
#include "response.hpp"
#include "signals.hpp"
#include <memory>

ClientConnection::ClientConnection() {}

ClientConnection::ClientConnection(
    std::shared_ptr<ServerConnection> ServerConnection)
    : ptrServerConnection(ServerConnection) {}

ClientConnection::~ClientConnection() {
  for (size_t i = 0; i < _connectedClients.size(); ++i) {
    logClientConnection("connection closed", _connectedClients[i].clientIP,
                        _connectedClients[i].clientFD);
    close(_connectedClients[i].clientFD);
  }
}

int ClientConnection::getIndexByClientFD(int clientFD) {
  int index = 0;
  for (size_t i = 0; i < _connectedClients.size(); i++) {
    if (_connectedClients[i].clientFD == clientFD)
      index = i;
  }
  return (index);
}

void ClientConnection::manageKeepAlive(int index) {
  int indexConnectedClients =
      getIndexByClientFD(_serverClientSockets[index].fd);
  time_t currentTime;
  time(&currentTime);
  _connectedClients[indexConnectedClients].lastRequestTime = currentTime;
  _connectedClients[indexConnectedClients].numRequests += 1;
  if (_connectedClients[indexConnectedClients].keepAlive == true)
    _serverClientSockets[index].events = POLLIN | POLLOUT;
  else
    removeClientSocket(_serverClientSockets[index].fd);
}

void ClientConnection::handleInputEvent(int index) {
  char buffer[1024];
  uint32_t connectedClientFD = getIndexByClientFD(index);

  ssize_t bytesRead =
      recv(_serverClientSockets[index].fd, buffer, sizeof(buffer), 0);
  if (bytesRead == -1) {
    logClientError("Failed to receive data from client",
                   _connectedClients[connectedClientFD].clientIP,
                   _serverClientSockets[index].fd);
    return;
  }
  if (bytesRead == 0 &&
      _connectedClients[connectedClientFD].keepAlive == false) {
    logClientError("Client disconnected",
                   _connectedClients[connectedClientFD].clientIP,
                   _serverClientSockets[index].fd);
    _serverClientSockets[index].revents = POLLERR;
    return;
  }

  buffer[bytesRead] = '\0';

  auto request = std::make_shared<Request>(buffer);
  // Use outcommented code to for parsing and sending response. If keepAlive is
  // set to false, the client will be disconnected. If keepAlive is set to true,
  // the client will stay connected till chunked request is done.
  // _connectedClients[connectedClientFD].keepAlive =
  // request.parseRequest(_connectedClients[getIndexByClientFD(index)]); // Fix
  // the error by using getIndexByClientFD(index) instead of connectedClientFD
//   std::cout << "Adress congig:" << _serverConfigs[index] << std::endl;
//   _serverConfigs[connectedClientFD]->show_self();
  Response response(request, *_connectedClients[connectedClientFD]._config); //changed to get server config

  const std::string httpResponse =
      response.get_response(); //"here is the response from the server\n";
  ssize_t bytesSent = send(_serverClientSockets[index].fd, httpResponse.c_str(),
                           httpResponse.length(), 0);
  if (bytesSent == -1) {
    std::cerr << "Failed to send data to client: " << strerror(errno)
              << std::endl;
    return;
  } else if (bytesSent == 0) {
    logClientError(
        "Client disconnected",
        _connectedClients[getIndexByClientFD(_serverClientSockets[index].fd)]
            .clientIP,
        _serverClientSockets[index].fd);
    _serverClientSockets[index].revents = POLLERR;
  }
  manageKeepAlive(index);
}

void ClientConnection::addSocketsToPollfdContainer() {
  for (size_t i = 0; i < ptrServerConnection->_connectedServers.size(); i++) {
    pollfd server_pollfd;
    server_pollfd.fd = ptrServerConnection->_connectedServers[i].serverFD;
    server_pollfd.events = POLLIN;
    _serverClientSockets.push_back(server_pollfd);
	_serverConfigs.push_back(ptrServerConnection->_connectedServers[i]._config); // addded
  }
  for (size_t i = 0; i < _connectedClients.size(); ++i) {
    pollfd client_pollfd;
    client_pollfd.fd = _connectedClients[i].clientFD;
    client_pollfd.events = POLLIN;
    _serverClientSockets.push_back(client_pollfd);
	_serverConfigs.push_back(ptrServerConnection->_connectedServers[i]._config); // addded this is poop just for synchronisation
  }
}

ClientInfo ClientConnection::initClientInfo(int clientFD,
                                            sockaddr_in clientAddr) {
  ClientInfo clientInfo;
  memset(&clientInfo, 0, sizeof(clientInfo));
  time_t currentTime;
  time(&currentTime);
  inet_ntop(AF_INET, &clientAddr.sin_addr, clientInfo.clientIP,
            sizeof(clientInfo.clientIP));
  clientInfo.clientFD = clientFD;
  clientInfo.keepAlive = false;
  clientInfo.timeOut = 100; // will be configurable later
  clientInfo.lastRequestTime = currentTime;
  clientInfo.numRequests = 0; // can be perhaps be deleted
  clientInfo.maxRequests = 3; // will be configurable later
  return clientInfo;
}

void ClientConnection::acceptClients(int serverFD, int index) {
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  int clientFD =
      accept(serverFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
  if (clientFD == -1) {
    logError("Failed to connect on server");
    return;
  }
  if (getpeername(clientFD, (struct sockaddr *)&clientAddr, &clientAddrLen) !=
      0)
    logError("Failed to read client IP");
  _connectedClients.push_back(initClientInfo(clientFD, clientAddr));
  _connectedClients.back()._config = _serverConfigs[index]; //_serverClientSockets[serverFD]; // added
  logClientConnection("accepted connection", _connectedClients.back().clientIP,
                      clientFD);
}

void ClientConnection::removeClientSocket(int clientFD) {
  close(clientFD);
  logClientConnection("closed connection",
                      _connectedClients[getIndexByClientFD(clientFD)].clientIP,
                      clientFD);
  int indexConnectedClients = getIndexByClientFD(clientFD);
  _connectedClients.erase(indexConnectedClients + _connectedClients.begin());
  int	i = 0; //added
  for (auto it = _serverClientSockets.begin(); it != _serverClientSockets.end();
       ++it) {
    if (it->fd == clientFD) {
      _serverClientSockets.erase(it); //?
	  _serverConfigs.erase(_serverConfigs.begin() + i); //added
      break;
    }
	i++; // added
  }
}

bool ClientConnection::isServerSocket(int fd) {
  for (const auto &server_fd : ptrServerConnection->_connectedServers) {
    if (fd == server_fd.serverFD)
      return true;
  }
  return false;
}

void ClientConnection::handlePollOutEvent(size_t index) {
  _serverClientSockets[index].events &= ~POLLOUT;
}

void ClientConnection::handlePollErrorEvent(size_t index) {
  removeClientSocket(_serverClientSockets[index].fd);
}

void ClientConnection::checkConnectedClientsStatus() {
  time_t currentTime;
  time(&currentTime);
  for (size_t i = 0; i < _connectedClients.size(); i++) {
    if (currentTime - _connectedClients[i].lastRequestTime >
            _connectedClients[i].timeOut &&
        _connectedClients[i].keepAlive == true)
      removeClientSocket(_connectedClients[i].clientFD);
    if (_connectedClients[i].numRequests >= _connectedClients[i].maxRequests &&
        _connectedClients[i].keepAlive == true)
      removeClientSocket(_connectedClients[i].clientFD);
  }
}

void ClientConnection::setUpClientConnection() {
  while (true) {
    _serverClientSockets.clear();
	_serverConfigs.clear(); //added
    addSocketsToPollfdContainer();
    int poll_count =
        poll(_serverClientSockets.data(), _serverClientSockets.size(), 100);
    checkConnectedClientsStatus();
    if (poll_count > 0) {
      for (size_t i = 0; i < _serverClientSockets.size(); i++) {
        if (_serverClientSockets[i].revents & POLLIN) {
          if (isServerSocket(_serverClientSockets[i].fd))
            acceptClients(_serverClientSockets[i].fd, i);
          else
            handleInputEvent(i);
        }
        if (_serverClientSockets[i].revents & POLLOUT)
          handlePollOutEvent(i);
        if (_serverClientSockets[i].revents & (POLLHUP | POLLERR)) {
          handlePollErrorEvent(i);
          // i--;
        }
      }
    } 
    else if (poll_count == 0)
      continue;
    else if (globalSignalReceived == 1) {
		  logAdd("Signal received, closing server connection");
		  break;
   }
    else
      logError("Failed to poll");
	}
}
