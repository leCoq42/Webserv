#pragma once

#include "ServerConnection.hpp"
<<<<<<< HEAD
#include "ServerStruct.hpp" //added
=======
#include "ServerStruct.hpp"
#include "request.hpp"
#include "response.hpp"
#include "log.hpp"
#include "signals.hpp"
>>>>>>> 7a7ade1fc9ad20d14c5ff2a3c04f950a752e1ef0
#include <memory>
#include <sys/poll.h>
#include <sys/types.h>
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <sys/socket.h>
#include <system_error>
#include <algorithm>

#define SENDING 0

struct ConnectionInfo {
    int clientFD;
    char clientIP[INET_ADDRSTRLEN];
    int port;
    std::shared_ptr<Request> request;
    std::shared_ptr<Response> response;
    long int timeOut;
    long int lastRequestTime;
    std::string receiveStr;
    std::string responseStr;
    int sendStatus;
    int totalBytesSent;
    int bytesToSend;
    pollfd pfd;
};

class ClientConnection {
private:
    std::shared_ptr<ServerConnection> _ptrServerConnection;
    std::map<int, ConnectionInfo> _connectionInfo;
	Log _log;

public:
    ClientConnection();
    ClientConnection(std::shared_ptr<ServerConnection> serverConnection);
    ~ClientConnection();

    void handlePollInEvent(int clientFD);
    void acceptClients(int server_fd, int index);
    void initializeServerSockets();
    void setupClientConnection(std::list<ServerStruct> *serverStruct);
    void removeClientSocket(int clientFD);
    bool isServerSocket(int fd);
    void handlePollOutEvent(int clientFD, std::list<ServerStruct> *serverStruct);
    void handlePollErrorEvent(int clientFD);
    void addClientInfo(int clientFD, int index, sockaddr_in clientAddr);
    void checkConnectedClientsStatus();
    void receiveData(int clientFD);
    bool initializeRequest(int clientFD);
    bool clientHasTimedOut(int clientFD);
    void sendData(int clientFD);
};