#pragma once

#include "ServerStruct.hpp"
#include "log.hpp"
#include <arpa/inet.h> //to convert ip into string
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#define BACKLOG 10

struct ServerInfo {
  std::string serverID;
  int serverFD;
  int serverPort;
  struct sockaddr_in server_addr;
  ServerStruct			*_config; //added server config
};

class ServerConnection : public virtual Log {
private:
public:
  std::vector<ServerInfo> _connectedServers;
//   ServerStruct			_config;
  ServerConnection();
  ~ServerConnection();

  void initServerInfo(ServerStruct &serverStruct, ServerInfo &info,
                      std::list<std::string>::iterator it);
  void createServerSocket(ServerInfo &info);
  void bindServerSocket(ServerInfo &info);
  void listenIncomingConnections(ServerInfo &info);
  void setUpServerConnection(ServerStruct &serverStruct);
};
