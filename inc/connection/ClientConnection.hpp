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


struct clientInfo {
	char						clientIP[INET_ADDRSTRLEN];
	int							clientFD;
	bool						isFileUpload;
	uint32_t					expectedContentLength;// check int!
	std::shared_ptr<Request>	request;
	bool						unchunking;
	long int					timeOut;
	long int					lastRequestTime;
	ServerStruct				*_config; //port/server config for multiple server setup
	Chunked						unchunker; //unchunker object to save multipart requests into an bufferfile
	std::string					buff_str;
	ssize_t						totalBytesReceived; //buffered amount
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
		void		handlePollOutEvent(int index);
		void		handlePollErrorEvent(int index);
		clientInfo	initClientInfo(int clientFD, int index, sockaddr_in clientAddr);
		// void		manageClientInfo(int polledFDIndex, int activeClientsIndex);
		void		checkConnectedClientsStatus();
		int 		findClientIndex(int clientFD);
		ssize_t		receiveData(int index, int activeClientsIndex);
		bool		initializeRequest(int activeClientsIndex);
		bool		clientHasTimedOut(int polledFdsIndex, int activeClientsIndex);
		void		sendData(int polledIndex, Response response);
};

