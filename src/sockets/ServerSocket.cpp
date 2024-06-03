#include "ServerSocket.hpp"

ServerSocket::ServerSocket() {
}


ServerSocket::~ServerSocket() {
    for (size_t i = 0; i < _vecServerSockets.size(); i++) {
        #ifdef DEBUG
            std::cout << "Closing server socket listening on port " << _vecServerSockets[i] << std::endl;
        #endif
        close(_vecServerSockets[i]);
    }
}

struct sockaddr_in ServerSocket::defineServerAddress(std::list<std::string>::iterator it) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(it->c_str()));
    return server_addr;
}

int ServerSocket::createServerSocket() {
    int serverSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_fd == -1) {
        throw std::runtime_error(std::string("Error creating server socket: ") + std::strerror(errno));
    }
    return (serverSocket_fd);
}

void ServerSocket::bindServerSocket(int serverSocket_fd, struct sockaddr_in &server_addr) {
    const int reuse = 1;
    if (setsockopt(serverSocket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) != 0) {
        close(serverSocket_fd);
        throw std::runtime_error(std::string("Could not configure socket options: ") + std::strerror(errno));
    }
    if (bind(serverSocket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(serverSocket_fd);
        throw std::runtime_error("Error binding socket: " + std::string(std::strerror(errno)));
    }
}

void ServerSocket::listenIncomingConnections(int serverSocket_fd) {
	if (listen(serverSocket_fd, 5) == -1) { // Adjust the value (5) based on the performance and scalability needs.
		close(serverSocket_fd);
		throw std::runtime_error(std::string("Error listening for connection on server socket: ") + std::to_string(serverSocket_fd) + std::string(std::strerror(errno)));
	}
}

void ServerSocket::setUpServerSockets(ServerStruct &serverinfo) {
    std::list<std::string>::iterator it = serverinfo.port.content_list.begin();
    try {
    	for (; it != serverinfo.port.content_list.end(); ++it) {
    	    struct sockaddr_in server_addr = defineServerAddress(it);
    	    int serverSocket_fd = createServerSocket();
    	    bindServerSocket(serverSocket_fd, server_addr);
    	    listenIncomingConnections(serverSocket_fd);
			#ifdef DEBUG
    	      std::cout << "Initial server socket listening on port " << atoi(it->c_str()) << std::endl;
			#endif
    	    _vecServerSockets.push_back(serverSocket_fd);
    	}
	}
	catch (const std::exception &e){
		std::cerr << e.what() << std::endl;
	}
}