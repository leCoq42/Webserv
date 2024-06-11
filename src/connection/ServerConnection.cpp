#include "ServerConnection.hpp"

ServerSocket::ServerSocket() {
}


ServerSocket::~ServerSocket() {
    for (size_t i = 0; i < _connectedServers.size(); i++) {
        #ifdef DEBUG
            std::cout << "Closing server socket listening on port " << _connectedServers[i].serverFD << std::endl;
        #endif
        close(_connectedServers[i].serverFD);
    }
}

void    ServerSocket::initServerInfo(ServerStruct serverStruct, ServerInfo& info, std::list<std::string>::iterator it) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    
    info.serverPort = atoi(it->c_str());
    info.serverID = serverStruct.id;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(it->c_str()));
    info.server_addr = server_addr;
}

void    ServerSocket::createServerSocket(ServerInfo &info) {
    info.serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (info.serverFD == -1)
        throw std::runtime_error(std::string("Error creating server socket: ") + std::strerror(errno));
}

void ServerSocket::bindServerSocket(ServerInfo &info) {
    const int reuse = 1;
    if (setsockopt(info.serverFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) != 0) {
        close(info.serverFD);
        throw std::runtime_error(std::string("Could not configure socket options: ") + std::strerror(errno));
    }
    if (bind(info.serverFD, (struct sockaddr *)&info.server_addr, sizeof(info.server_addr)) == -1) {
        close(info.serverFD);
        throw std::runtime_error("Error binding socket: " + std::string(std::strerror(errno)));
    }
}

void ServerSocket::listenIncomingConnections(ServerInfo &info) {
	if (listen(info.serverFD, 5) == -1) { // Adjust the value (5) based on the performance and scalability needs.
		close(info.serverFD);
		throw std::runtime_error(std::string("Error listening for connection on server socket: ") + std::to_string(info.serverFD) + std::string(std::strerror(errno)));
	}
}

void ServerSocket::setUpServerSockets(ServerStruct serverStruct) {
    std::list<std::string>::iterator it= serverStruct.port.content_list.begin();
    for (; it!= serverStruct.port.content_list.end(); ++it) {
        if (atoi(it->c_str()) > 0 && atoi(it->c_str()) < 65536) {
                ServerInfo info;
                initServerInfo(serverStruct, info, it);
                createServerSocket(info);
                bindServerSocket(info);
                listenIncomingConnections(info);
                _connectedServers.push_back(info);
                #ifdef DEBUG
                    std::cout << "Initial server socket listening on port " << atoi(it->c_str()) << std::endl;
                #endif
        }
        else {
            #ifdef DEBUG
                std::cerr << "Invalid port number: " << *it << std::endl;
            #endif
        }
    }
}