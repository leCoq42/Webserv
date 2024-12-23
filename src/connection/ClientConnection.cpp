#include "ClientConnection.hpp"
#include "defines.hpp"
#include <csignal>
#include <sys/poll.h>

ClientConnection::ClientConnection(std::shared_ptr<ServerConnection> ServerConnection, std::shared_ptr<Log> log)
	: _ptrServerConnection(ServerConnection), _log(log) {
	}

ClientConnection::~ClientConnection() {
	for (auto it: _connectionInfo) {
		if (!isServerSocket(it.second.pfd.fd)) {
			if (it.second.pfd.fd >= 0) {
				close(it.second.pfd.fd);
				it.second.pfd.fd = -1;
			}
		}
	}
}

void ClientConnection::handlePollErrorEvent(int clientFD) {
	removeClientSocket(clientFD);
}

void ClientConnection::handlePollOutEvent(int clientFD, std::list<ServerStruct> *serverStruct)
{
	if (_connectionInfo.find(clientFD) == _connectionInfo.end() || _connectionInfo[clientFD].pfd.fd < 1)
		return;
	if (contentTooLarge(clientFD, serverStruct))
		return;

	auto& client = _connectionInfo[clientFD];
	if (!client.response) {
		client.response = std::make_shared<Response>(client.request, serverStruct, client.port, _log);
		client.responseStr = client.response->get_response();
		client.bytesToSend = client.response->get_response().length();
	}
	else if (client.response && client.response->isComplete() == false)
		client.response->continue_cgi();
	else {
		client.responseStr = client.response->get_response();
		client.bytesToSend = client.response->get_response().length();
		sendData(clientFD);
	}
}

bool ClientConnection::clientHasTimedOut(int clientFD, std::list<ServerStruct> *serverStruct)
{
	if (clientFD < 1 || _connectionInfo[clientFD].pfd.fd < 1)
		return false;
	auto& client = _connectionInfo[clientFD];
	time_t currentTime;
	time(&currentTime);

	if (client.lastPacketTime && (currentTime - client.lastPacketTime > client.timeOut)) {
		_log->logClientConnection("Client timed out", client.clientIP, clientFD);
		client.response = std::make_shared<Response>(504, "Gateway Timeout", serverStruct, client.port, _log);
		client.responseStr = client.response->get_response();
		client.bytesToSend = client.response->get_response().length();
		client.totalBytesSent = 0;
		client.pfd.events = POLLOUT;
		return true;
	}
	return false;
}

bool ClientConnection::contentTooLarge(int clientFD, std::list<ServerStruct> *serverStruct)
{
	auto& client = _connectionInfo[clientFD];

	if (!client.request)
		return false;

	if (client.request->get_body().size() > client.maxBodySize) {
		_log->logClientConnection("Request: Content Too large", client.clientIP, clientFD);
		client.response = std::make_shared<Response>(413, "Content Too Large", serverStruct, client.port, _log);
		client.responseStr = client.response->get_response();
		client.bytesToSend = client.response->get_response().length();
		client.totalBytesSent = 0;
		sendData(clientFD);
		return true;
	}
	return false;
}

void ClientConnection::sendData(int clientFD)
{
	if (clientFD < 1 || _connectionInfo[clientFD].pfd.fd < 1)
		return;
	auto& clientInfo = _connectionInfo[clientFD];
	int remainingBytes = clientInfo.bytesToSend - clientInfo.totalBytesSent;
	int packageSize = std::min(BUFFSIZE, remainingBytes);
	ssize_t bytesSent = send(clientFD,
							 clientInfo.responseStr.c_str() + clientInfo.totalBytesSent, packageSize, 0);
	if (bytesSent > 0) {
		clientInfo.totalBytesSent += bytesSent;
		if (clientInfo.totalBytesSent < clientInfo.bytesToSend) {
			return;
		}
	}
	if (bytesSent < 0) {
		_log->logClientError("Failed to send data to client: " + std::string(strerror(errno)), clientInfo.clientIP, clientFD);
		removeClientSocket(clientFD);
		return;
	}
	else {
		removeClientSocket(clientFD);
		#ifdef DEBUG
		_log->logClientConnection("Client removed.", clientInfo.clientIP, clientFD);
		#endif
	}
}

bool ClientConnection::initializeRequest(int clientFD)
{
	auto& client = _connectionInfo[clientFD];
	time_t  currentTime;
	time(&currentTime);
	size_t  headerEnd = client.receiveStr.find(CRLFCRLF);

	if (headerEnd != std::string::npos) {
		client.request = std::make_shared<Request>(client.receiveStr.substr(0, headerEnd + CRLFCRLFsize), _log);
		if (headerEnd + CRLFCRLFsize < client.receiveStr.length())
			client.request->appendToBody(client.receiveStr.substr(headerEnd + CRLFCRLFsize));
		client.receiveStr.clear();
		client.lastPacketTime = currentTime;
		return true;
	}
	return false;
}

void ClientConnection::receiveData(int clientFD)
{
	auto& client = _connectionInfo[clientFD];
	std::vector<char> buffer(BUFFSIZE);

	ssize_t bytesReceived = recv(clientFD, &buffer[0], buffer.size(), MSG_DONTWAIT);
	if (bytesReceived > 0) {
		client.receiveStr.append(std::string(buffer.begin(), buffer.begin() + bytesReceived));
		time(&client.lastPacketTime);
	}
	else if (bytesReceived < 0) {
		_log->logClientError("Failed to receive data from client: " + std::string(std::strerror(errno)),
				client.clientIP, clientFD);
		removeClientSocket(clientFD);
	}
	else
	{
		removeClientSocket(clientFD);
		#ifdef DEBUG
		_log->logClientConnection("Client disconnected.", clientInfo.clientIP, clientFD);
		#endif
	}
}

void ClientConnection::handlePollInEvent(int clientFD)
{
	if (_connectionInfo[clientFD].pfd.fd < 1)
		return;
	receiveData(clientFD);
	auto& client = _connectionInfo[clientFD];
	if (!client.request) {
		if (!initializeRequest(clientFD))
			return;
	}
	else {
		client.request->appendToBody(client.receiveStr);
		client.receiveStr.clear();
	}
	if (client.request->get_requestStatus() == true) {
		client.lastPacketTime = 0;
		client.pfd.events = POLLOUT;
	}
}

void ClientConnection::initClientInfo(int clientFD, sockaddr_in clientAddr, ServerInfo server) {
	ConnectionInfo clientInfo;
	time_t currentTime;
	time(&currentTime);

	inet_ntop(AF_INET, &clientAddr.sin_addr, clientInfo.clientIP, sizeof(clientInfo.clientIP));
	clientInfo.port = server.serverPort;
	clientInfo.maxBodySize = server.MaxBodySize;
	clientInfo.request = nullptr;
	clientInfo.response = nullptr;
	clientInfo.timeOut = TIMEOUT;
	clientInfo.lastPacketTime = 0;
	clientInfo.receiveStr = "";
	clientInfo.responseStr = "";
	clientInfo.totalBytesSent = 0;
	clientInfo.bytesToSend = 0;
	clientInfo.pfd.fd = clientFD;
	clientInfo.pfd.events = POLLIN;
	_connectionInfo[clientFD] = clientInfo;
}

ServerInfo ClientConnection::findServerInfo(int serverFD)
{
	for (const auto &server : _ptrServerConnection->_connectedPorts) {
		if (serverFD == server.serverFD)
			return server;
	}
	return *_ptrServerConnection->_connectedPorts.begin();
}

void ClientConnection::acceptClients(int serverFD)
{
	ServerInfo serverInfo = findServerInfo(serverFD);
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientFD = accept(serverFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
	if (clientFD == -1) {
		_log->logError("Failed to connect client: " + std::string(strerror(errno)));
		return;
	}
	if (getpeername(clientFD, (struct sockaddr *)&clientAddr, &clientAddrLen) != 0) {
		_log->logError("Failed to read client IP: " + std::string(strerror(errno)));
		close(clientFD);
		_connectionInfo[clientFD].pfd.fd = -1;
		return;
	}
	initClientInfo(clientFD, clientAddr, serverInfo);
	time(&_connectionInfo[clientFD].lastPacketTime);
	#ifdef DEBUG
	_log->logClientConnection("accepted connection", _connectionInfo[clientFD].clientIP, clientFD);
	#endif
}

bool ClientConnection::isServerSocket(int fd)
{
	for (const auto &server_fd : _ptrServerConnection->_connectedPorts) {
		if (fd == server_fd.serverFD)
		return true;
	}
	return false;
}

void ClientConnection::removeClientSocket(int clientFD)
{
	if (_connectionInfo.find(clientFD) == _connectionInfo.end())
		return;
	else if (_connectionInfo[clientFD].pfd.fd > 1)
		close(clientFD);
	#ifdef DEBUG
	_log->logClientConnection("closed connection", _connectionInfo[clientFD].clientIP, clientFD);
	#endif
	_connectionInfo[clientFD].pfd.fd = -1;
}

void ClientConnection::initServerSockets() {
	for (const auto& server : _ptrServerConnection->_connectedPorts) {
		ConnectionInfo serverInfo;

		serverInfo.clientIP[0] = '\0';
		serverInfo.port = server.serverPort;
		serverInfo.maxBodySize = server.MaxBodySize;
		serverInfo.request = nullptr;
		serverInfo.response = nullptr;
		serverInfo.timeOut = 0;
		serverInfo.lastPacketTime = 0;
		serverInfo.receiveStr = "";
		serverInfo.responseStr = "";
		serverInfo.totalBytesSent = 0;
		serverInfo.bytesToSend = 0;
		serverInfo.pfd.fd = server.serverFD;
		serverInfo.pfd.events = POLLIN;
		_connectionInfo[server.serverFD] = serverInfo;
	}
}

void ClientConnection::setupClientConnection(std::list<ServerStruct> *serverStruct)
{
	initServerSockets();

	while (true) {
		std::vector<pollfd> pollfds;
		for (const auto& connection : _connectionInfo) {
			if (connection.second.pfd.fd > 0){ //hacky patch
				pollfds.push_back(connection.second.pfd);
				if (connection.second.response && connection.second.response->get_cgi())
				{
					pollfds.push_back(connection.second.response->get_cgi()->get_pollfdRead());
					pollfds.push_back(connection.second.response->get_cgi()->get_pollfdWrite());
				}
			}
		}

		int poll_count = poll(pollfds.data(), pollfds.size(),TIMEOUT * 1000);
		// std::cout << _connectionInfo.size() << std::endl;
		if (poll_count > 0) {
			std::vector<pollfd>::iterator pfd_it = pollfds.begin();
			while (pfd_it < pollfds.end()){
				int		it_with = 1;
				pollfd	pfd = *pfd_it;
				if (_connectionInfo[pfd.fd].response && _connectionInfo[pfd.fd].response->get_cgi())
				{
					_connectionInfo[pfd.fd].response->get_cgi()->get_pollfdRead().revents = (pfd_it + 1)->revents;
					_connectionInfo[pfd.fd].response->get_cgi()->get_pollfdWrite().revents = (pfd_it + 2)->revents;
					it_with = 3;
				}
				if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL))
					handlePollErrorEvent(pfd.fd);
				else if (pfd.revents & POLLIN){
					if (isServerSocket(pfd.fd))
						acceptClients(pfd.fd);
					else
						handlePollInEvent(pfd.fd);
				}
				// else if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL))
				// 	handlePollErrorEvent(pfd.fd);
				else if (pfd.revents & POLLOUT && (it_with != 3 || 
					(_connectionInfo[pfd.fd].response->get_cgi()->get_pollfdRead().revents || _connectionInfo[pfd.fd].response->get_cgi()->get_pollfdWrite().revents || _connectionInfo[pfd.fd].response->get_cgi()->isComplete())))
					handlePollOutEvent(pfd.fd, serverStruct);
				pfd_it += it_with;
			}
		}
		for (const auto& pfd : pollfds)
			clientHasTimedOut(pfd.fd, serverStruct);
		if (globalSignalReceived == 1) {
			_log->logAdd("Interrupt signal received.");
			break;
		}
		else if (poll_count < 0)
			_log->logError("Failed to poll.");
	}
}
