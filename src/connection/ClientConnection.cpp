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
#include <algorithm>

ClientConnection::ClientConnection() {}

ClientConnection::ClientConnection(std::shared_ptr<ServerConnection> ServerConnection)
    : _ptrServerConnection(ServerConnection) {}

ClientConnection::~ClientConnection()
{
    for (size_t i = 0; i < _activeClients.size(); ++i) {
        logClientConnection("connection closed", _activeClients[i].clientIP,
                      _activeClients[i].clientFD);
        close(_activeClients[i].clientFD);
        _activeClients[i].unchunker.close_file();
    }
}

void ClientConnection::handlePollErrorEvent(int polledFdIndex) {
    removeClientSocket(_polledFds[polledFdIndex].fd);
}

int ClientConnection::findClientIndex(int clientFD)
{
    for (size_t i = 0; i < _activeClients.size(); i++) {
        if (_activeClients[i].clientFD == clientFD)
            return (i); 
    }
    return (-1);
}

ssize_t ClientConnection::receiveData(int polledFdsIndex, int activeClientsIndex)
{
    std::vector<char> buffer(BUFFSIZE);
    ssize_t bytesReceived = recv(_polledFds[polledFdsIndex].fd, &buffer[0], buffer.size(), MSG_DONTWAIT);
    if (bytesReceived > 0)
        _activeClients[activeClientsIndex].buff_str.append(std::string(buffer.begin(), buffer.begin() + bytesReceived));
    if (bytesReceived < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
        logClientError("Failed to receive data from client: " + std::string(std::strerror(errno)),
                _activeClients[activeClientsIndex].clientIP, _polledFds[polledFdsIndex].fd);
        removeClientSocket(_polledFds[polledFdsIndex].fd);
    }
    if (bytesReceived == 0) {
        logClientConnection("Client disconnected", _activeClients[activeClientsIndex].clientIP, _polledFds[polledFdsIndex].fd);
        removeClientSocket(_polledFds[polledFdsIndex].fd);
    }
    return (bytesReceived);
}

void ClientConnection::sendData(int polledFdsIndex, Response response)
{
    const std::string responseString = response.get_response();
    ssize_t bytesSent = send(_polledFds[polledFdsIndex].fd, responseString.c_str(), responseString.length(), 0);
    if (bytesSent == -1) {
        removeClientSocket(_polledFds[polledFdsIndex].fd);
        logClientError("Failed to send data to client: " + std::string(strerror(errno)),_activeClients[findClientIndex(_polledFds[polledFdsIndex].fd)].clientIP, _polledFds[polledFdsIndex].fd);
    }
    else if (bytesSent == 0) {
        removeClientSocket(_polledFds[polledFdsIndex].fd);
        logClientConnection("Client disconnected", _activeClients[findClientIndex(_polledFds[polledFdsIndex].fd)].clientIP, _polledFds[polledFdsIndex].fd);
    }
}

bool ClientConnection::initializeRequest(int activeClientsIndex) 
{
    time_t currentTime;
    time(&currentTime);
    size_t headerEnd = _activeClients[activeClientsIndex].buff_str.find(CRLFCRLF);
    if (headerEnd != std::string::npos) {
        _activeClients[activeClientsIndex].request = 
            std::make_shared<Request>(_activeClients[activeClientsIndex].buff_str.substr(0, headerEnd + 4));
        if (headerEnd + 4 < _activeClients[activeClientsIndex].buff_str.length()) {
            _activeClients[activeClientsIndex].request->appendToBody(
                _activeClients[activeClientsIndex].buff_str.substr(headerEnd + 4));
        }
        _activeClients[activeClientsIndex].buff_str.clear();
        _activeClients[activeClientsIndex].lastRequestTime = currentTime;
        return (true);
    } 
    return (false);
}

bool ClientConnection::clientHasTimedOut(int polledFdsIndex, int activeClientsIndex) 
{
    time_t currentTime;
    time(&currentTime);
    if (currentTime - _activeClients[activeClientsIndex].lastRequestTime > _activeClients[activeClientsIndex].timeOut) {
        logClientConnection("Client timed out", _activeClients[activeClientsIndex].clientIP, _activeClients[activeClientsIndex].clientFD);
        // Response response(*_activeClients[activeClientsIndex]._config, 408); // todo: doesnt work
        // sendData(polledFdsIndex, response);
        removeClientSocket(_polledFds[polledFdsIndex].fd);
        return (true);
    }
    return (false);
}

void ClientConnection::handleInputEvent(int polledFdsIndex, std::list<ServerStruct> *serverStruct) 
{
    int activeClientsIndex = findClientIndex(_polledFds[polledFdsIndex].fd);
    if (activeClientsIndex == -1)
        return;
    if (clientHasTimedOut(polledFdsIndex, activeClientsIndex) == true)
        return;
    ssize_t bytesReceived = receiveData(polledFdsIndex, activeClientsIndex);
    if (bytesReceived < 0 && (errno != EAGAIN && errno != EWOULDBLOCK))
        return;
    if (bytesReceived == 0)
        return;
    if (!_activeClients[activeClientsIndex].request) {
        if (!initializeRequest(activeClientsIndex))
            return;
    } 
    else {
        _activeClients[activeClientsIndex].request->appendToBody(_activeClients[activeClientsIndex].buff_str);
        _activeClients[activeClientsIndex].buff_str.clear();
    }
    if (_activeClients[activeClientsIndex].request->get_requestStatus() == true) {
        Response response(_activeClients[activeClientsIndex].request, serverStruct, _activeClients[activeClientsIndex].port); // serverStruct
        sendData(polledFdsIndex, response);
        removeClientSocket(_polledFds[polledFdsIndex].fd);
    }
}

clientInfo ClientConnection::initClientInfo(int clientFD, int serverIndex, sockaddr_in clientAddr) {
    clientInfo info;
    time_t currentTime;
    time(&currentTime);
    
    inet_ntop(AF_INET, &clientAddr.sin_addr, info.clientIP, sizeof(info.clientIP));
    info.clientFD = clientFD;
    info.port = _ptrServerConnection->_connectedServers[serverIndex].serverPort;
    info.isFileUpload = false;
    info.expectedContentLength = 0;
    info.request = nullptr;
    info.unchunking = false;
    info.timeOut = 10;
    info.lastRequestTime = currentTime;
    // info._config = _serverConfigs[serverIndex];
    info.buff_str = "";
    info.totalBytesReceived = 0;

    return info;
}

void ClientConnection::acceptClients(int serverFD, int serverIndex) 
{
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientFD = accept(serverFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientFD == -1) {
        logError("Failed to connect client: " + std::string(strerror(errno)));
        return;
    }
    
    // Set socket to non-blocking mode
    int flags = fcntl(clientFD, F_GETFL, 0);
    fcntl(clientFD, F_SETFL, flags | O_NONBLOCK);
    
    if (getpeername(clientFD, (struct sockaddr *)&clientAddr, &clientAddrLen) != 0) {
        logError("Failed to read client IP: " + std::string(strerror(errno)));
        close(clientFD);
        return;
    }
    _activeClients.push_back(initClientInfo(clientFD, serverIndex, clientAddr));
    
    pollfd client_pollfd;
    client_pollfd.fd = clientFD;
    client_pollfd.events = POLLIN | POLLOUT;
    _polledFds.push_back(client_pollfd);
    
    logClientConnection("accepted connection", _activeClients.back().clientIP, clientFD);
}

void ClientConnection::removeClientSocket(int clientFD)
{
    if (_activeClients.empty())
        return;
    int activeClientIndex = findClientIndex(clientFD);
    if (activeClientIndex == -1)
        return;
    
    close(clientFD);
    logClientConnection("closed connection", _activeClients[activeClientIndex].clientIP, clientFD);
    _activeClients.erase(_activeClients.begin() + activeClientIndex);

    // Remove from _polledFds
    _polledFds.erase(
        std::remove_if(_polledFds.begin(), _polledFds.end(),
            [clientFD](const pollfd& pfd) { return pfd.fd == clientFD; }),
        _polledFds.end()
    );
}

bool ClientConnection::isServerSocket(int fd)
{
    for (const auto &server_fd : _ptrServerConnection->_connectedServers) {
        if (fd == server_fd.serverFD)
        return (true);
    }
    return (false);
}

void ClientConnection::handlePollOutEvent(int polledFdsIndex)
{
    _polledFds[polledFdsIndex].events &= ~POLLOUT;
}

void ClientConnection::checkConnectedClientsStatus() 
{
    time_t currentTime;
    time(&currentTime);
    for (size_t i = 0; i < _activeClients.size(); i++) {
        if (currentTime - _activeClients[i].lastRequestTime >
            _activeClients[i].timeOut)
                removeClientSocket(_activeClients[i].clientFD);
    }
}

void ClientConnection::initializeServerSockets() {
    for (const auto& server : _ptrServerConnection->_connectedServers) {
        pollfd server_pollfd;
        server_pollfd.fd = server.serverFD;
        server_pollfd.events = POLLIN | POLLOUT;
        _polledFds.push_back(server_pollfd);
    }
}

void ClientConnection::setupClientConnection(std::list<ServerStruct> *serverStruct)
{
    initializeServerSockets(); // Initialize server sockets once

    while (true) {
        int poll_count = poll(_polledFds.data(), _polledFds.size(), 100);
        checkConnectedClientsStatus();
        if (poll_count > 0) {
            for (size_t index = 0; index < _polledFds.size(); index++) {
                if (_polledFds[index].revents & POLLIN) {
                    if (isServerSocket(_polledFds[index].fd)) 
                        acceptClients(_polledFds[index].fd, index);
                    else
                        handleInputEvent(index, serverStruct);
                }
                if (_polledFds[index].revents & POLLOUT)
                    handlePollOutEvent(index);
                if (_polledFds[index].revents & (POLLHUP | POLLERR))
                    handlePollErrorEvent(index);
            }
        }
        if (globalSignalReceived == 1) {
            logAdd("Signal received, closing server connection");
            break;
        }
        else if (poll_count < 0)
            logError("Failed to poll.");
    }
}