#pragma once

#include "ServerConnection.hpp"
#include "ServerStruct.hpp" //added
#include "Chunked.hpp"
#include <memory>
#include <sys/poll.h>
#include <sys/types.h>
#include <vector>
#include "request.hpp"
#include "response.hpp"
#include <fcntl.h>
#include <unistd.h>
#include "log.hpp"


struct clientInfo {
	int							clientFD;
	char						clientIP[INET_ADDRSTRLEN];
	int							port;
	std::shared_ptr<Request>	request;
	std::shared_ptr<Response>	response;
	long int					timeOut;
	long int					lastRequestTime;
	std::string					receiveStr;
	std::string					responseStr;
	int							sendStatus;
	int							totalBytesSent;
	int							bytesToSend;
};

#define SENDING 0

class ClientConnection : ServerConnection {
	private:
		std::shared_ptr<ServerConnection>	_ptrServerConnection;
		std::vector<clientInfo>				_activeClients;
		std::vector<pollfd>					_polledFds;

	public:
		ClientConnection();
		ClientConnection(std::shared_ptr<ServerConnection> serverConnection);
		~ClientConnection();

	//TODO: Zijn veel van deze functies niet const?
		void		handlePollInEvent(int polledFdsIndex);
		void		acceptClients(int server_fd, int index);
		void		initializeServerSockets();
		void 		setupClientConnection(std::list<ServerStruct> *serverStruct);
		void		removeClientSocket(int clientFD);
		bool		isServerSocket(int fd);
		void		handlePollOutEvent(int index, std::list<ServerStruct> *serverStruct);
		void		handlePollErrorEvent(int index);
		clientInfo	initClientInfo(int clientFD, int index, sockaddr_in clientAddr);
		// void		manageClientInfo(int polledFDIndex, int activeClientsIndex);
		void		checkConnectedClientsStatus();
		int 		findClientIndex(int clientFD);
		void		receiveData(int index, int activeClientsIndex);
		bool		initializeRequest(int activeClientsIndex);
		bool		clientHasTimedOut(int polledFdsIndex, int activeClientsIndex);
		void		sendData(int polledIndex, int activeClientsIndex);
};
