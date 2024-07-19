#include "ClientConnection.hpp"
#include "request.hpp"
#include "response.hpp"
#include "signals.hpp"
#include "defines.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>

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
	if (_activeClients[indexConnectedClients].keepAlive == true)
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

ssize_t ClientConnection::receiveData(int index, std::string &datareceived)
{
	std::vector<char> buffer(BUFFSIZE);
	int	connectedClientFD = getIndexByClientFD(_pollFdsWithConfigs[index].fd);
	ssize_t bytes_received;
	std::string::size_type total_received = 0;

	while (true) {
		bytes_received = recv(_pollFdsWithConfigs[index].fd, &buffer[0], buffer.size(), MSG_DONTWAIT);
		if (bytes_received > 0)
		{
			total_received += bytes_received;
			datareceived.append(std::string(buffer.begin(), buffer.begin() + bytes_received));
		}
		else if (bytes_received < 0 && (errno != EAGAIN || errno != EWOULDBLOCK))
		{
			logClientError("Failed to receive data from client: " + std::string(std::strerror(errno)),
						_activeClients[connectedClientFD].clientIP,
						_pollFdsWithConfigs[index].fd);
			return -1;
		}
		else
			break;
	}
	return total_received;
}

void ClientConnection::handleInputEvent(int index)
{
	std::string	buffer_str;
	int			connectedClientFD = getIndexByClientFD(_pollFdsWithConfigs[index].fd);
	ssize_t		bytesRead;
	ssize_t		totalBytes;

	totalBytes = receiveData(index, buffer_str);
	if (totalBytes <= 0)
		return;

	std::shared_ptr<Request> request = std::make_shared<Request>(buffer_str);
	if (!request)
		return; // error?

	while (request->get_requestStatus() == requestStatus::INCOMPLETE) {
		buffer_str.clear();
		bytesRead = receiveData(index, buffer_str);
		if (bytesRead < 0 && (errno != EAGAIN || errno != EWOULDBLOCK)) {
			logClientError("Failed to receive data from client: " + std::string(std::strerror(errno)),
				  _activeClients[connectedClientFD].clientIP,_pollFdsWithConfigs[index].fd);
			return;
		}
		if (bytesRead == 0)
			break;
		request->appendToBody(buffer_str);
		totalBytes += bytesRead;
	}

	std::cout << MSG_BORDER << "[Total bytes received: " << totalBytes << "]" << MSG_BORDER << std::endl;
	std::cout << MSG_BORDER << "[Total Bodylength: " << request->get_body().length() << "]" << MSG_BORDER << std::endl;

	Response response(request, *_activeClients[connectedClientFD]._config);

	const std::string responseString = response.get_response();
	ssize_t bytesSent = send(_pollFdsWithConfigs[index].fd, responseString.c_str(),
							responseString.length(), 0);
	if (bytesSent == -1) {
		logClientError(
			"Failed to send data to client: " + std::string(strerror(errno)),
			_activeClients[getIndexByClientFD(_pollFdsWithConfigs[index].fd)]
				.clientIP,
			_pollFdsWithConfigs[index].fd);
	}
	else if (bytesSent == 0) {
		logClientConnection(
			"Client disconnected",
			_activeClients[getIndexByClientFD(_pollFdsWithConfigs[index].fd)]
				.clientIP,
			_pollFdsWithConfigs[index].fd);
	}

	reset_buffer(_activeClients[connectedClientFD], true);
	if (bytesSent < 0 || bytesSent == 0)
		return;
	std::cout << MSG_BORDER << "[Total Bytes Send: " << bytesSent << "]" << MSG_BORDER << std::endl;
	_activeClients[connectedClientFD].keepAlive = request->get_keepAlive(); //keep alive as in header
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
