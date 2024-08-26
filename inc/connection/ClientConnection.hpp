#pragma once

#include "ServerConnection.hpp"
#include "ServerStruct.hpp"
#include "request.hpp"
#include "response.hpp"
#include "log.hpp"
#include "signals.hpp"
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
#define TIMEOUT 30

struct ConnectionInfo {
	int							FD;
	char						clientIP[INET_ADDRSTRLEN];
	int							port;
	size_t					  	maxBodySize;
	std::shared_ptr<Request>	request;
	std::shared_ptr<Response>	response;
	long int 					timeOut;
	long int 					lastRequestTime;
	std::string 				receiveStr;
	std::string					responseStr;
	int 						totalBytesSent;
	int 						bytesToSend;
	pollfd         				pfd;
};

class ClientConnection {
private:
	std::shared_ptr<ServerConnection>   _ptrServerConnection;
	std::map<int, ConnectionInfo>       _connectionInfo;
	std::shared_ptr<Log>                _log;

public:
	ClientConnection(std::shared_ptr<ServerConnection> serverConnection, std::shared_ptr<Log> log);
	~ClientConnection();

	void	setupClientConnection(std::list<ServerStruct> *serverStruct);
	void	handlePollInEvent(int clientFD, std::list<ServerStruct> *serverStruct);
	void	acceptClients(int serverFD);
	void	initServerSockets();
	void	removeClientSocket(int clientFD);
	bool	isServerSocket(int fd);
	void	handlePollOutEvent(int clientFD, std::list<ServerStruct> *serverStruct);
	void	handlePollErrorEvent(int clientFD);
	void	initClientInfo(int clientFD, sockaddr_in clientAddr, ServerInfo server);
	void	receiveData(int clientFD);
	bool	initializeRequest(int clientFD);
	bool	clientHasTimedOut(int clientFD, std::list<ServerStruct> *serverStruct);
	bool	contentTooLarge(int clientFD, std::list<ServerStruct> *serverStruct);
	void	sendData(int clientFD);
	ServerInfo	findServerInfo(int serverFD);
};
