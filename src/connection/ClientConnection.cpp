#include "ClientConnection.hpp"

ClientConnection::ClientConnection(std::shared_ptr<ServerConnection> ServerConnection, std::shared_ptr<Log> log)
	: _ptrServerConnection(ServerConnection), _log(log) {
	}

ClientConnection::~ClientConnection() {
	for (auto it: _connectionInfo) {
		if (!isServerSocket(it.first)) {
			if (it.first > 0) {
				close(it.first);
			}
		}
	}
}

void ClientConnection::handlePollErrorEvent(int clientFD) {
	removeClientSocket(clientFD);
}

void ClientConnection::handlePollOutEvent(int clientFD, std::list<ServerStruct> *serverStruct)
{	
	if (_connectionInfo[clientFD].FD == 0)
		return;
	if (clientHasTimedOut(clientFD, serverStruct))
		return;
	auto& client = _connectionInfo[clientFD];

	if (!client.response) {
		client.response = std::make_shared<Response>(client.request, serverStruct, client.port, _log);
		client.responseStr = client.response->get_response();
		client.bytesToSend = client.response->get_response().length();
	}
	else if (client.response && client.response->isComplete() == false) {
		client.response->continue_cgi();
	}
	else {
		client.responseStr = client.response->get_response();
		client.bytesToSend = client.response->get_response().length();
		sendData(clientFD);
	}
}



bool ClientConnection::clientHasTimedOut(int clientFD, std::list<ServerStruct> *serverStruct)
{
	auto& client = _connectionInfo[clientFD];
	time_t currentTime;
	time(&currentTime);

	if (currentTime - client.lastRequestTime > client.timeOut) {
		_log->logClientConnection("Client timed out", client.clientIP, clientFD);
		client.response = std::make_shared<Response>(504, "Gateway Timeout", serverStruct, client.port, _log);
		client.responseStr = client.response->get_response();
		client.bytesToSend = client.response->get_response().length();
		sendData(clientFD);
		removeClientSocket(clientFD);
		return true;
	}
	return false;
}

void ClientConnection::sendData(int clientFD)
{
	if (_connectionInfo[clientFD].FD == 0)
		return;
	auto& clientInfo = _connectionInfo[clientFD];
	int remainingBytes = clientInfo.bytesToSend - clientInfo.totalBytesSent;
	int packageSize = std::min(BUFFSIZE, remainingBytes);

	if (_connectionInfo.find(clientFD) == _connectionInfo.end())
		return;
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
		#ifdef DEBUG
		_log->logClientConnection("Client disconnected", clientInfo.clientIP, clientFD);
		#endif
		removeClientSocket(clientFD);
	}
}

bool ClientConnection::initializeRequest(int clientFD)
{
	auto& client = _connectionInfo[clientFD];
	time_t  currentTime;
	time(&currentTime);
	size_t  headerEnd = client.receiveStr.find(CRLFCRLF);
	size_t  sizeCRLFCRLF = strlen(CRLFCRLF);

	if (headerEnd != std::string::npos) {
		client.request = std::make_shared<Request>(client.receiveStr.substr(0, headerEnd + sizeCRLFCRLF), _log);
		if (headerEnd + sizeCRLFCRLF < client.receiveStr.length()) {
			client.request->appendToBody(client.receiveStr.substr(headerEnd + sizeCRLFCRLF));
		}
		client.receiveStr.clear();
		client.lastRequestTime = currentTime;
		return true;
	}
	return false;
}

void ClientConnection::receiveData(int clientFD)
{
	auto& client = _connectionInfo[clientFD];
	std::vector<char> buffer(BUFFSIZE);

	ssize_t bytesReceived = recv(clientFD, &buffer[0], buffer.size(), MSG_DONTWAIT);
	if (bytesReceived > 0)
		client.receiveStr.append(std::string(buffer.begin(), buffer.begin() + bytesReceived));
	if (bytesReceived < 0) {
		_log->logClientError("Failed to receive data from client: " + std::string(std::strerror(errno)),
				client.clientIP, clientFD);
		removeClientSocket(clientFD);
	}
}

void ClientConnection::handlePollInEvent(int clientFD, std::list<ServerStruct> *serverStruct)
{
	if (_connectionInfo[clientFD].FD == 0)
		return;
	if (clientHasTimedOut(clientFD, serverStruct))
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
		client.pfd.events = POLLOUT;
	}
}

void ClientConnection::initClientInfo(int clientFD, sockaddr_in clientAddr, ServerInfo server) {
	ConnectionInfo clientInfo;
	time_t currentTime;
	time(&currentTime);

	clientInfo.FD = clientFD;
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientInfo.clientIP, sizeof(clientInfo.clientIP));
	clientInfo.port = server.serverPort;
	clientInfo.maxBodySize = server.MaxBodySize;
	clientInfo.request = nullptr;
	clientInfo.response = nullptr;
	clientInfo.timeOut = TIMEOUT;
	clientInfo.lastRequestTime = currentTime;
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
	for (const auto &server : _ptrServerConnection->_connectedServers) {
		if (serverFD == server.serverFD)
			return server;
	}
	return *_ptrServerConnection->_connectedServers.begin();
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
		return;
	}
	initClientInfo(clientFD, clientAddr, serverInfo);
	#ifdef DEBUG
	_log->logClientConnection("accepted connection", _connectionInfo[clientFD].clientIP, clientFD);
	#endif
}

bool ClientConnection::isServerSocket(int fd)
{
	for (const auto &server_fd : _ptrServerConnection->_connectedServers) {
		if (fd == server_fd.serverFD)
		return true;
	}
	return false;
}

void ClientConnection::removeClientSocket(int clientFD)
{
	if (_connectionInfo.find(clientFD) == _connectionInfo.end())
		return;

	close(clientFD);
	#ifdef DEBUG
	_log->logClientConnection("closed connection", _connectionInfo[clientFD].clientIP, clientFD);
	#endif
	_connectionInfo.erase(clientFD);
	_connectionInfo[clientFD].FD = 0;
}

void ClientConnection::initServerSockets() {
	for (const auto& server : _ptrServerConnection->_connectedServers) {
		ConnectionInfo serverInfo;

		serverInfo.FD = server.serverFD;
		serverInfo.clientIP[0] = '\0';
		serverInfo.port = server.serverPort;
		serverInfo.maxBodySize = server.MaxBodySize;
		serverInfo.request = nullptr;
		serverInfo.response = nullptr;
		serverInfo.timeOut = 0;
		serverInfo.lastRequestTime = 0;
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
		for (const auto& client : _connectionInfo) {
			pollfds.push_back(client.second.pfd);
		}

		int poll_count = poll(pollfds.data(), pollfds.size(), 10);
		if (poll_count > 0) {
			for (const auto& pfd : pollfds) {
				if (pfd.revents & POLLIN) {
					if (isServerSocket(pfd.fd))
						acceptClients(pfd.fd);
					else
						handlePollInEvent(pfd.fd, serverStruct);
				}
				else if (pfd.revents & POLLOUT)
					handlePollOutEvent(pfd.fd, serverStruct);
				if (pfd.revents & (POLLHUP | POLLERR)) {
					std::cout << "poll error" << std::endl;
					handlePollErrorEvent(pfd.fd);
				}
			}
		}
		if (globalSignalReceived == 1) {
			_log->logAdd("Interrupt signal received.");
			break;
		}
		else if (poll_count < 0)
			_log->logError("Failed to poll.");
		if (poll_count == 0)
			continue;
	}
}
