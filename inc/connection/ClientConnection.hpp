#pragma once

#include "ServerConnection.hpp"
#include "ServerStruct.hpp" //added
#include "Chunked.hpp"
#include <memory>
#include <sys/poll.h>
#include <sys/types.h>
#include <vector>

struct clientInfo {
	char			clientIP[INET_ADDRSTRLEN];
	int				clientFD;
	bool			keepAlive;
	bool			unchunking;
	long int		timeOut;
	long int		lastRequestTime;
	size_t			numRequests;
	size_t			maxRequests;
	ServerStruct	*_config; //port/server config for multiple server setup
	Chunked			unchunker; //unchunker object to save multipart requests into an bufferfile
	char			buffer[1024*100]; //buffer without blocking, in combination with maxnumrequests limits the upload size
	ssize_t			bytesRead = 0; //buffered amount
};

class ClientConnection : ServerConnection, public virtual Log {
	private:
		std::shared_ptr<ServerConnection>	_ptrServerConnection;
		std::vector<clientInfo>				_activeClients;
		std::vector<pollfd>					_polledFds;
		std::vector<ServerStruct*>			_serverConfigs; //added synchronous to pollfd vector

	public:
		ClientConnection();
		ClientConnection(std::shared_ptr<ServerConnection> serverConnection);
		~ClientConnection();

	//TODO: Zijn veel van deze functies niet const?
		void		handleInputEvent(int index);
		void		acceptClients(int server_fd, int index);
		void		addSocketsToPollfdContainer();
		void		setupClientConnection();
		void		removeClientSocket(int clientFD);
		bool		isServerSocket(int fd);
		void		handlePollOutEvent(size_t index);
		void		handlePollErrorEvent(size_t index);
		clientInfo	initClientInfo(int clientFD, int index, sockaddr_in clientAddr);
		void		manageKeepAlive(int index);
		void		checkConnectedClientsStatus();
		int 		findClientIndex(int clientFD);
		void		receiveData(int index, std::string &datareceived, int activeClientsIndex);
		void		sendData(int polledIndex, Request request, int activeClientsIndex);
};

