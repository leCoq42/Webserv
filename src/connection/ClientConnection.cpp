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
  removeClientSocket(_polledFds[index].fd);
}

int ClientConnection::findClientIndex(int clientFD)
{
	int activeClientsIndex = 0;
	for (size_t i = 0; i < _activeClients.size(); i++)
	{
		if (_activeClients[i].clientFD == clientFD)
			activeClientsIndex = i;
	}
	return (activeClientsIndex);
}

void ClientConnection::manageKeepAlive(int polledFdIndex) {
	int indexActiveClients = findClientIndex(_polledFds[polledFdIndex].fd);
	time_t currentTime;
	time(&currentTime);
	_activeClients[indexActiveClients].lastRequestTime = currentTime;
	_activeClients[indexActiveClients].numRequests += 1;
	if (_activeClients[indexActiveClients].keepAlive == true || _activeClients[indexActiveClients].unchunking == true)
	{
		std::cout << "REPOLL" << std::endl;
		_polledFds[polledFdIndex].events = POLLIN | POLLOUT;
	}
	else
	{
		std::cout << "REMOVED_CLIENT" << std::endl;
		removeClientSocket(_polledFds[polledFdIndex].fd);
	}
}

void	reset_buffer(clientInfo &client, bool end_of_request)
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

void ClientConnection::receiveData(int index, std::string &datareceived, int activeClientsIndex)
{
	std::vector<char> buffer(BUFFSIZE);
	ssize_t bytes_received;
	std::string::size_type total_received = 0;

		bytes_received = recv(_polledFds[index].fd, &buffer[0], buffer.size(), MSG_DONTWAIT);
		if (bytes_received > 0)
		{
			total_received += bytes_received;
			_activeClients[activeClientsIndex].bytesRead += bytes_received;
			datareceived.append(std::string(buffer.begin(), buffer.begin() + bytes_received));
		}
		else if (bytes_received < 0 && (errno != EAGAIN || errno != EWOULDBLOCK))
		{
			logClientError("Failed to receive data from client: " + std::string(std::strerror(errno)),
						_activeClients[activeClientsIndex].clientIP,
						_polledFds[index].fd);
		}
	// test print total bytes received and buffer vector
	std::cout << MSG_BORDER << "[Total bytes received: " << total_received << "]" << MSG_BORDER << std::endl;
}

void	ClientConnection::sendData(int polledFdsIndex, Request request, int activeClientsIndex)
{
	// std::cout << MSG_BORDER << "[Total Bodylength: " << request->get_body().length() << "]" << MSG_BORDER << std::endl;
	Response response(request, *_activeClients[activeClientsIndex]._config);
	const std::string responseString = response.get_response();
	ssize_t bytesSent = send(_polledFds[polledFdsIndex].fd, responseString.c_str(),
							responseString.length(), 0);
	if (bytesSent == -1) 
	{
		logClientError(
			"Failed to send data to client: " + std::string(strerror(errno)),
			_activeClients[findClientIndex(_polledFds[polledFdsIndex].fd)]
				.clientIP,
			_polledFds[polledFdsIndex].fd);
		// return reset_buffer(_activeClients[activeClientsIndex], true);
	}
	else if (bytesSent == 0) {
		logClientConnection(
			"Client disconnected",
			_activeClients[findClientIndex(_polledFds[polledFdsIndex].fd)]
				.clientIP,
			_polledFds[polledFdsIndex].fd);
		// return reset_buffer(_activeClients[activeClientsIndex], true);
	}

	std::cout << MSG_BORDER << "[Total Bytes Send: " << bytesSent << "]" << MSG_BORDER << std::endl;
	// reset_buffer(_activeClients[activeClientsIndex], true);
	_activeClients[activeClientsIndex].keepAlive = request->get_keepAlive(); //keep alive as in header
}

//Intended workings
// When an request is incomplete and goes through handlemultipart and is not completely received. Chunked object has to save the initial request, since sequential requests don't have headers.
// and the response should be hold off? Until the last sequential request is received and passed through Chunked.add_to_file(). When that happens Chunked object will set _totalLength on true. Indicating that the full body was received. 
// Then the response should be build again with the bufferedfile given so handlemultipart will have the whole body. Request could also be build with the whole body, body size might be too big for request object at the moment.
// When the response is build again after Chunked _totalLength is true. Response will have acces to the full request BODY (without headers and boundaries) through the given filename. CGI should be able to be run then as well.
// I think the filename has to be given to argv, (might be happening already).
void ClientConnection::handleInputEvent(int polledFdsIndex)
{
	std::string	buffer_str;
	int			activeClientsIndex = findClientIndex(_polledFds[polledFdsIndex].fd);

	receiveData(polledFdsIndex, buffer_str, activeClientsIndex);
	std::shared_ptr<Request> request = std::make_shared<Request>(buffer_str); // resetten?
	if (!request)
		return; // error?
	if (request->get_requestStatus() != requestStatus::COMPLETE) 
	{
		request->appendToBody(buffer_str);
		return;
	}
	sendData(polledFdsIndex, request, activeClientsIndex);
	manageKeepAlive(polledFdsIndex);
}

clientInfo	ClientConnection::initClientInfo(int clientFD, int index, sockaddr_in clientAddr)
{
	clientInfo info;
	memset(&info, 0, sizeof(info));
	time_t currentTime;
	time(&currentTime);
	inet_ntop(AF_INET, &clientAddr.sin_addr, info.clientIP,
		   sizeof(info.clientIP));
	info.clientFD = clientFD;
	info.keepAlive = false;
	info.timeOut = 30; // will be configurable later, limits upload size // What is 30? Seconds?
	info.lastRequestTime = currentTime;
	info.numRequests = 0; // can be perhaps be deleted
	info.maxRequests = std::numeric_limits<int>::max(); // will be configurable later, yes missed this one might have to be more then this (chunked takes them off so will limit total upload size)
	info._config = _serverConfigs[index];
	return info;
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
						_activeClients[findClientIndex(clientFD)].clientIP, clientFD);
	int indexConnectedClients = findClientIndex(clientFD);
	_activeClients[indexConnectedClients].unchunker.close_file();
	_activeClients.erase(indexConnectedClients + _activeClients.begin());
	for (auto it = _polledFds.begin(); it != _polledFds.end(); ++it)
	{
		if (it->fd == clientFD)
		{
			_polledFds.erase(it);
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
  _polledFds[index].events &= ~POLLOUT;
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
		_polledFds.push_back(server_pollfd);
		_serverConfigs.push_back(_ptrServerConnection->_connectedServers[i]._config); // addded
	}
	for (size_t i = 0; i < _activeClients.size(); ++i) {
		pollfd client_pollfd;
		client_pollfd.fd = _activeClients[i].clientFD;
		client_pollfd.events = POLLIN | POLLOUT;
		_polledFds.push_back(client_pollfd);
	}
}

void ClientConnection::setupClientConnection()
{
	while (true)
	{
		_polledFds.clear();
		_serverConfigs.clear(); //added
		addSocketsToPollfdContainer();
		int poll_count = poll(_polledFds.data(), _polledFds.size(), 100);
		checkConnectedClientsStatus();
		if (poll_count > 0)
		{
			for (size_t index = 0; index < _polledFds.size(); index++)
			{
				if (_polledFds[index].revents & POLLIN)
				{
					if (isServerSocket(_polledFds[index].fd)) 
						acceptClients(_polledFds[index].fd, index);
					else
						handleInputEvent(index);
				}
				if (_polledFds[index].revents & POLLOUT)
					handlePollOutEvent(index);
				if (_polledFds[index].revents & (POLLHUP | POLLERR))
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
