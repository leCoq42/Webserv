#pragma once

#include "ServerConnection.hpp"
#include "ServerStruct.hpp" //added
#include "Chunked.hpp"
#include <memory>
#include <sys/poll.h>
#include <vector>

//TODO: Waarom staat dit niet in de/een class zelf? -> Client.hpp?
struct ClientInfo {
  char			clientIP[INET_ADDRSTRLEN];
  int			clientFD;
  bool			keepAlive;
  bool			unchunking;
  long int		timeOut;
  long int		lastRequestTime;
  size_t		numRequests;
  size_t		maxRequests;
  ServerStruct	*_config; //port/server config for multiple server setup
  Chunked		unchunker; //unchunker object to save multipart requests into an bufferfile
  char			buffer[1024*100]; //buffer without blocking, in combination with maxnumrequests limits the upload size
  ssize_t		bytesRead = 0; //buffered amount
};

class ClientConnection : ServerConnection, public virtual Log {
	private:
		std::shared_ptr<ServerConnection>	_ptrServerConnection;
		std::vector<ClientInfo>				_connectedClients;
		std::vector<pollfd>					_serverClientSockets;
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
		ClientInfo	initClientInfo(int clientFD, sockaddr_in clientAddr);
		void		manageKeepAlive(int index);
		void		checkConnectedClientsStatus();
		int 		getIndexByClientFD(int clientFD);
};
