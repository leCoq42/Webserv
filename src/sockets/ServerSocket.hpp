#ifndef ServerSocket_HPP
#define ServerSocket_HPP

#include <arpa/inet.h> //to convert ip into string
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

//
#include "../src/parser_0/inc/parser.hpp" //
#include <stdio.h>                        /* printf, fgets */
#include <stdlib.h>                       /* atoi */

#define BACKLOG 10

class ServerSocket {
private:
  // struct sockaddr_in	_server_addr;
  std::vector<int> _vecServerSockets;

public:
  ServerSocket();
  ~ServerSocket();

  int createServerSocket();
  struct sockaddr_in defineServerAddress(ServerStruct &serverinfo);
  void bindServerSocket(int server_fd, struct sockaddr_in &server_addr);
  void listenIncomingConnections(int server_fd);
  int setUpServerSocket(ServerStruct &serverinfo);
};

#endif
