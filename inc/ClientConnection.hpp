#pragma once

#include "ServerConnection.hpp"
#include <memory>
#include <sys/poll.h>
#include <vector>

struct ClientInfo {
  char clientIP[INET_ADDRSTRLEN];
  int clientFD;
  bool keepAlive;
  long int timeOut;
  long int lastRequestTime;
  size_t numRequests;
  size_t maxRequests;
};

class ClientConnection : ServerConnection, public virtual Log {
private:
  std::shared_ptr<ServerConnection> ptrServerConnection;
  std::vector<ClientInfo> _connectedClients;
  std::vector<pollfd> _serverClientSockets;

public:
  ClientConnection();
  ClientConnection(std::shared_ptr<ServerConnection> serverConnection);
  ~ClientConnection();

  void handleInputEvent(int index);
  void acceptClients(int server_fd);
  void addSocketsToPollfdContainer();
  void setUpClientConnection();
  void removeClientSocket(int clientFD);
  bool isServerSocket(int fd);
  void handlePollOutEvent(size_t index);
  void handlePollErrorEvent(size_t index);
  ClientInfo initClientInfo(int clientFD, sockaddr_in clientAddr);
  void manageKeepAlive(int index);
  void checkConnectedClientsStatus();
  int getIndexByClientFD(int clientFD);
};
