#include "ClientConnection.hpp"
#include "request.hpp"
#include "response.hpp"
#include "signals.hpp"
#include <cerrno>
#include <memory>

ClientConnection::ClientConnection() {}

ClientConnection::ClientConnection(std::shared_ptr<ServerConnection> ServerConnection)
    : _ptrServerConnection(ServerConnection) {}

ClientConnection::~ClientConnection()
{
	for (size_t i = 0; i < _activeClients.size(); ++i)
	{
		logClientConnection("connection closed", _activeClients[i].clientIP,
					  _activeClients[i].clientFD);
		close(_activeClients[i].clientFD);
		_activeClients[i].unchunker.close_file();
	}
}
void ClientConnection::handlePollErrorEvent(size_t index) {
  removeClientSocket(_pollFdsWithConfigs[index].fd);
}

int ClientConnection::getIndexByClientFD(int clientFD)
{
	int index = 0;
	for (size_t i = 0; i < _activeClients.size(); i++)
	{
		if (_activeClients[i].clientFD == clientFD)
			index = i;
	}
	return (index);
}

void ClientConnection::manageKeepAlive(int index) {
	int indexConnectedClients =
		getIndexByClientFD(_pollFdsWithConfigs[index].fd);
	time_t currentTime;
	time(&currentTime);
	_activeClients[indexConnectedClients].lastRequestTime = currentTime;
	_activeClients[indexConnectedClients].numRequests += 1;
	if (_activeClients[indexConnectedClients].keepAlive == true || _activeClients[indexConnectedClients].unchunking == true)
	{
		std::cout << "REPOLL" << std::endl;
		_pollFdsWithConfigs[index].events = POLLIN | POLLOUT;
	}
	else
	{
		std::cout << "REMOVED_CLIENT" << std::endl;
		removeClientSocket(_pollFdsWithConfigs[index].fd);
	}
}

void	reset_buffer(Client &client, bool end_of_request)
{
	client.buffer[0] = 0;
	client.bytesRead = 0;
	if (end_of_request)
	{
		client.unchunking = false;
		client.unchunker.close_file();
		client.unchunker = Chunked();
	}
}

//Intended workings
// When an request is incomplete and goes through handlemultipart and is not completely received. Chunked object has to save the initial request, since sequential requests don't have headers.
// and the response should be hold off? Until the last sequential request is received and passed through Chunked.add_to_file(). When that happens Chunked object will set _totalLength on true. Indicating that the full body was received. 
// Then the response should be build again with the bufferedfile given so handlemultipart will have the whole body. Request could also be build with the whole body, body size might be too big for request object at the moment.
// When the response is build again after Chunked _totalLength is true. Response will have acces to the full request BODY (without headers and boundaries) through the given filename. CGI should be able to be run then as well.
// I think the filename has to be given to argv, (might be happening already).
void ClientConnection::handleInputEvent(int index) {
	std::string	buffer_str;
	uint32_t	connectedClientFD = getIndexByClientFD(_pollFdsWithConfigs[index].fd);
	ssize_t		bytesRead = _activeClients[connectedClientFD].bytesRead;

	int n = 0;
	errno = 0;
	n = recv(_pollFdsWithConfigs[index].fd, &_activeClients[connectedClientFD].buffer[bytesRead], sizeof(_activeClients[connectedClientFD].buffer) - bytesRead, MSG_DONTWAIT);
	buffer_str = "";
	if (n >= 0)
	{
		bytesRead += n;
		_activeClients[connectedClientFD].bytesRead = bytesRead;
		_activeClients[connectedClientFD].buffer[bytesRead] = '\0';
		buffer_str = _activeClients[connectedClientFD].buffer;
	}
	else if (n < 0 && errno != EAGAIN)
	{
		logClientError("Failed to receive data from client",
					_activeClients[connectedClientFD].clientIP,
					_pollFdsWithConfigs[index].fd);
		return;
	}
	else
		return ;

	if (buffer_str.find("\r\n\r\n") == std::string::npos && !_activeClients[connectedClientFD].unchunking)
		return ;
	_activeClients[connectedClientFD].unchunking = false;

	if (bytesRead == 0 &&
		((_activeClients[connectedClientFD].keepAlive == true) ||
		!_activeClients[connectedClientFD].unchunker._totalLength))
	{
		logClientError("Client disconnected",
					_activeClients[connectedClientFD].clientIP,
					_pollFdsWithConfigs[index].fd);
		_pollFdsWithConfigs[index].revents = POLLERR;
		std::cout << "CLIENT DISCONNECTED" << std::endl;
		reset_buffer(_activeClients[connectedClientFD], true);
		//close file and delete client?
		return;
	}

	std::string upload_file = "";
	if (!_activeClients[connectedClientFD].unchunker._totalLength && !_activeClients[connectedClientFD].unchunker._justStarted)
	{
		_activeClients[connectedClientFD].unchunking = true;
		Request	request = Request(buffer_str);
		std::cout << "TOTAL LENGTH NOT REACHED" << request.get_validity() << std::endl;
		if (request.get_validity()) //hacky and most often okay, but not always...
		{
			logClientError("Post interrupted",
					_activeClients[connectedClientFD].clientIP,
					_pollFdsWithConfigs[index].fd);
			_pollFdsWithConfigs[index].revents = POLLERR;
			buffer_str = _activeClients[connectedClientFD].buffer;
			return reset_buffer(_activeClients[connectedClientFD], true); //might have to remove chunked and then just go one with the valid one
		}
		if (!_activeClients[connectedClientFD].unchunker.add_to_file(_activeClients[connectedClientFD].buffer, bytesRead))
			return reset_buffer(_activeClients[connectedClientFD], false);
		_activeClients[connectedClientFD].unchunking = false;
		upload_file = _activeClients[connectedClientFD].unchunker.get_fileName();
		buffer_str = _activeClients[connectedClientFD].unchunker._firstRequest->get_rawRequest();
	}

	std::shared_ptr<Request> request = std::make_shared<Request>(buffer_str);
	// Use outcommented code to for parsing and sending response. If keepAlive is
	// set to false, the client will be disconnected. If keepAlive is set to true,
	// the client will stay connected till chunked request is done.
	// _activeClients[connectedClientFD].keepAlive =
	// request.parseRequest(_activeClients[getIndexByClientFD(index)]); // Fix
	// the error by using getIndexByClientFD(index) instead of connectedClientFD
	//   std::cout << "Adress congig:" << _serverConfigs[index] << std::endl;
	// if (request->get_requestStatus() == status::COMPLETE)
	Response response(request, *_activeClients[connectedClientFD]._config, upload_file); //changed to get server config

	_activeClients[connectedClientFD].keepAlive = request->get_keepAlive(); //keep alive as in header
	std::cout << "REQUEST POST FILE:" << request->get_bufferFile() << std::endl;
	if (request->get_bufferFile().compare(""))
	{
		std::cout << "THERE IS A BUFFER_FILE\n";
		_activeClients[connectedClientFD].unchunker = Chunked(request);
		if (!_activeClients[connectedClientFD].unchunker._totalLength)
		{
			//skip response, right place?
			_activeClients[connectedClientFD].unchunking = true;
			std::cout<< "skip response WILL IT STAY ALIVE" << _activeClients.size() << std::endl;
			manageKeepAlive(index);
			std::cout<< "skip response IT STAYS ALIVE" << _activeClients.size() << std::endl;
			return reset_buffer(_activeClients[connectedClientFD], false);
		}
	}
	std::cout << "RESPONDING" << std::endl;
	_activeClients[connectedClientFD].unchunking = false;
	const std::string httpResponse = response.get_response(); //"here is the response from the server\n";
	ssize_t bytesSent = send(_pollFdsWithConfigs[index].fd, httpResponse.c_str(),
							httpResponse.length(), 0);
	if (bytesSent == -1)
	{
		std::cerr << "Failed to send data to client: " << strerror(errno)
				<< std::endl;
		return reset_buffer(_activeClients[connectedClientFD], true);
	}
	else if (bytesSent == 0)
	{
		logClientError(
			"Client disconnected",
			_activeClients[getIndexByClientFD(_pollFdsWithConfigs[index].fd)]
				.clientIP,
			_pollFdsWithConfigs[index].fd);
	}
	reset_buffer(_activeClients[connectedClientFD], true);
	manageKeepAlive(index);
}



Client ClientConnection::initClientInfo(int clientFD, int index, sockaddr_in clientAddr)
{
	Client clientInfo;
	memset(&clientInfo, 0, sizeof(clientInfo));
	time_t currentTime;
	time(&currentTime);
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientInfo.clientIP,
		   sizeof(clientInfo.clientIP));
	clientInfo.clientFD = clientFD;
	clientInfo.keepAlive = false;
	clientInfo.timeOut = 30; // will be configurable later, limits upload size // What is 30? Seconds?
	clientInfo.lastRequestTime = currentTime;
	clientInfo.numRequests = 0; // can be perhaps be deleted
	clientInfo.maxRequests = std::numeric_limits<int>::max(); // will be configurable later, yes missed this one might have to be more then this (chunked takes them off so will limit total upload size)
	clientInfo._config = _serverConfigs[index];
	return clientInfo;
}

void ClientConnection::acceptClients(int serverFD, int index) {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientFD = accept(serverFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
	if (clientFD == -1)
	{
		logError("Failed to connect on server");
		return;
	}
	if (getpeername(clientFD, (struct sockaddr *)&clientAddr, &clientAddrLen) != 0)
		logError("Failed to read client IP");
	_activeClients.push_back(initClientInfo(clientFD, index, clientAddr));
	logClientConnection("accepted connection", _activeClients.back().clientIP, clientFD);
}

void ClientConnection::removeClientSocket(int clientFD)
{
	if (_activeClients.empty())
		return ;
	close(clientFD);
	logClientConnection("closed connection",
						_activeClients[getIndexByClientFD(clientFD)].clientIP, clientFD);
	int indexConnectedClients = getIndexByClientFD(clientFD);
	_activeClients[indexConnectedClients].unchunker.close_file();
	_activeClients.erase(indexConnectedClients + _activeClients.begin());
	for (auto it = _pollFdsWithConfigs.begin(); it != _pollFdsWithConfigs.end(); ++it)
	{
		if (it->fd == clientFD)
		{
			_pollFdsWithConfigs.erase(it);
			_serverConfigs.erase(_serverConfigs.begin()); // nodig?
			break;
		}
	}
}

bool ClientConnection::isServerSocket(int fd)
{
	for (const auto &server_fd : _ptrServerConnection->_connectedServers)
	{
		if (fd == server_fd.serverFD)
		return true;
	}
	return false;
}

void ClientConnection::handlePollOutEvent(size_t index) {
  _pollFdsWithConfigs[index].events &= ~POLLOUT;
}


void ClientConnection::checkConnectedClientsStatus() {
	time_t currentTime;
	time(&currentTime);
	for (size_t i = 0; i < _activeClients.size(); i++)
	{
		if (currentTime - _activeClients[i].lastRequestTime >
			_activeClients[i].timeOut && _activeClients[i].keepAlive == true)
				removeClientSocket(_activeClients[i].clientFD);
		if (_activeClients[i].numRequests >= _activeClients[i].maxRequests &&
			_activeClients[i].keepAlive == true)
				removeClientSocket(_activeClients[i].clientFD);
	}
}

void ClientConnection::addSocketsToPollfdContainer()
{
	for (size_t i = 0; i < _ptrServerConnection->_connectedServers.size(); i++) {
		pollfd server_pollfd;
		server_pollfd.fd = _ptrServerConnection->_connectedServers[i].serverFD;
		server_pollfd.events = POLLIN | POLLOUT;
		_pollFdsWithConfigs.push_back(server_pollfd);
		_serverConfigs.push_back(_ptrServerConnection->_connectedServers[i]._config); // addded
	}
	for (size_t i = 0; i < _activeClients.size(); ++i) {
		pollfd client_pollfd;
		client_pollfd.fd = _activeClients[i].clientFD;
		client_pollfd.events = POLLIN | POLLOUT;
		_pollFdsWithConfigs.push_back(client_pollfd);
	}
}

void ClientConnection::setupClientConnection()
{
	while (true)
	{
		_pollFdsWithConfigs.clear();
		_serverConfigs.clear(); //added
		addSocketsToPollfdContainer();
		int poll_count = poll(_pollFdsWithConfigs.data(), _pollFdsWithConfigs.size(), 100);
		checkConnectedClientsStatus();
		if (poll_count > 0)
		{
			for (size_t index = 0; index < _pollFdsWithConfigs.size(); index++)
			{
				if (_pollFdsWithConfigs[index].revents & POLLIN)
				{
					if (isServerSocket(_pollFdsWithConfigs[index].fd)) 
						acceptClients(_pollFdsWithConfigs[index].fd, index);
					else
						handleInputEvent(index);
				}
				if (_pollFdsWithConfigs[index].revents & POLLOUT)
					handlePollOutEvent(index);
				if (_pollFdsWithConfigs[index].revents & (POLLHUP | POLLERR))
					handlePollErrorEvent(index);
			}
		}
		if (globalSignalReceived == 1)
		{
			logAdd("Signal received, closing server connection");
			break;
		}
		else if (poll_count < 0)
			logError("Failed to poll.");
	}
}
