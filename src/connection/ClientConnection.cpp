#include "ClientConnection.hpp"

ClientConnection::ClientConnection(std::shared_ptr<ServerConnection> ServerConnection, std::shared_ptr<Log> log)
	: _ptrServerConnection(ServerConnection), _log(log) {
	}

ClientConnection::~ClientConnection() {}

void ClientConnection::handlePollErrorEvent(int clientFD) {
	removeClientSocket(clientFD);
}

void ClientConnection::handlePollOutEvent(int clientFD, std::list<ServerStruct> *serverStruct)
{
	if (_connectionInfo.find(clientFD) == _connectionInfo.end())
		return;
	if (clientHasTimedOut(clientFD))
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

bool ClientConnection::clientHasTimedOut(int clientFD) 
{
	auto& client = _connectionInfo[clientFD];
	time_t currentTime;
	time(&currentTime);

	if (currentTime - client.lastRequestTime > client.timeOut) {
		_log->logClientConnection("Client timed out", client.clientIP, clientFD);
		removeClientSocket(clientFD);
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
	if (bytesReceived < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
		_log->logClientError("Failed to receive data from client: " + std::string(std::strerror(errno)),
				client.clientIP, clientFD);
		removeClientSocket(clientFD);
	}
}

void ClientConnection::sendData(int clientFD)
{
	auto& clientInfo = _connectionInfo[clientFD];
	int remainingBytes = clientInfo.bytesToSend - clientInfo.totalBytesSent;
	int packageSize = std::min(BUFFSIZE, remainingBytes);

	ssize_t bytesSent = send(clientFD, 
							 clientInfo.responseStr.c_str() + clientInfo.totalBytesSent, packageSize, 0);
	if (bytesSent > 0) {
		clientInfo.totalBytesSent += bytesSent;
		if (clientInfo.totalBytesSent < clientInfo.bytesToSend) {
			clientInfo.sendStatus = SENDING;
			return;
		}
	}
	if (bytesSent < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
		_log->logClientError("Failed to send data to client: " + std::string(strerror(errno)), clientInfo.clientIP, clientFD);
		removeClientSocket(clientFD);
	}
	else {
		#ifdef DEBUG
		_log->logClientConnection("Client disconnected", clientInfo.clientIP, clientFD);
		#endif
		removeClientSocket(clientFD);
	}
}

void ClientConnection::handlePollInEvent(int clientFD) 
{
	if (_connectionInfo.find(clientFD) == _connectionInfo.end())
		return;
	if (clientHasTimedOut(clientFD))
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

void ClientConnection::initClientInfo(int clientFD, sockaddr_in clientAddr) {
	ConnectionInfo clientInfo;
	time_t currentTime;
	time(&currentTime);
	
	clientInfo.clientFD = clientFD;
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientInfo.clientIP, sizeof(clientInfo.clientIP));
	clientInfo.port = _ptrServerConnection->_connectedServers[0].serverPort;
	clientInfo.request = nullptr;
	clientInfo.response = nullptr;
	clientInfo.timeOut = 10;
	clientInfo.lastRequestTime = currentTime;
	clientInfo.receiveStr = "";
	clientInfo.responseStr = "";
	clientInfo.sendStatus = -1;
	clientInfo.totalBytesSent = 0;
	clientInfo.bytesToSend = 0;
	clientInfo.pfd.fd = clientFD;
	clientInfo.pfd.events = POLLIN;

	_connectionInfo[clientFD] = clientInfo;
}

void ClientConnection::acceptClients(int serverFD) 
{
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientFD = accept(serverFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
	if (clientFD == -1) {
		_log->logError("Failed to connect client: " + std::string(strerror(errno)));
		return;
	}
	
	int flags = fcntl(clientFD, F_GETFL, 0);
	fcntl(clientFD, F_SETFL, flags | O_NONBLOCK);
	
	if (getpeername(clientFD, (struct sockaddr *)&clientAddr, &clientAddrLen) != 0) {
		_log->logError("Failed to read client IP: " + std::string(strerror(errno)));
		close(clientFD);
		return;
	}
	initClientInfo(clientFD, clientAddr);
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
}

void ClientConnection::checkConnectedClientsStatus() 
{
	time_t currentTime;
	time(&currentTime);

	for (auto it = _connectionInfo.begin(); it != _connectionInfo.end();) {
		if (currentTime - it->second.lastRequestTime > it->second.timeOut) {
			int FD = it->first;
			++it;
			if(!isServerSocket(FD))
				removeClientSocket(FD);
		}
		else {
			++it;
		}
	}
}

void ClientConnection::initServerSockets() {
	for (const auto& server : _ptrServerConnection->_connectedServers) {
		ConnectionInfo serverInfo;

		serverInfo.clientFD = server.serverFD;
		serverInfo.request = nullptr;
		serverInfo.response = nullptr;
		serverInfo.timeOut = 0;
		serverInfo.lastRequestTime = 0;
		serverInfo.receiveStr = "";
		serverInfo.responseStr = "";
		serverInfo.sendStatus = -1;
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
		checkConnectedClientsStatus();
		if (poll_count > 0) {
			for (const auto& pfd : pollfds) {
				if (pfd.revents & POLLIN) {
					if (isServerSocket(pfd.fd)) 
						acceptClients(pfd.fd);
					else
						handlePollInEvent(pfd.fd);
				}
				if (pfd.revents & POLLOUT)
					handlePollOutEvent(pfd.fd, serverStruct);
				if (pfd.revents & (POLLHUP | POLLERR))
					handlePollErrorEvent(pfd.fd);
			}
		}
		if (globalSignalReceived == 1) {
			_log->logAdd("Interrupt signal received.");
			break;
		}
		else if (poll_count < 0)
			_log->logError("Failed to poll.");
	}
}
