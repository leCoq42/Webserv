#include "ServerSocket.hpp"
#include <list>

ServerSocket::ServerSocket() {
}

ServerSocket::~ServerSocket() {
	for(size_t i = 0; i < _vecServerSockets.size(); i++) {
		std::cout << "Closing server socket listening on port " << _vecServerSockets[i] << std::endl;
		close(_vecServerSockets[i]);
	}
}

struct sockaddr_in	ServerSocket::defineServerAddress(std::list<std::string>::iterator it) {
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;    
	server_addr.sin_port = htons(atoi(it->c_str()));
	return (server_addr);
}

int	ServerSocket::createServerSocket() {
	int	serverSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket_fd == -1) {
		std::cerr << "Error creating socket" << std::endl;
		exit(EXIT_FAILURE);
	}
	return (serverSocket_fd);
}

void	ServerSocket::bindServerSocket(int serverSocket_fd, struct sockaddr_in &server_addr) {
	const int reuse = 1;
	if (setsockopt(serverSocket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) != 0) {
		std::cerr << "Could not configure socket options: " + std::string(std::strerror(errno));
	}
	if (bind(serverSocket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		std::cerr << "Error binding socket" << std::endl;
		close(serverSocket_fd);
		exit(EXIT_FAILURE);
	}
}

void	ServerSocket::listenIncomingConnections(int serverSocket_fd) {
	if (listen(serverSocket_fd, 5) == -1) { // adjust the value (5) based on the performance and scalability needs.
		std::cerr << "Error listening for connection on server socket" << std::endl;
		close(serverSocket_fd);
	}
}

void	ServerSocket::setUpServerSockets(ServerStruct &serverinfo) {
	std::list<std::string>::iterator it = serverinfo.port.content_list.begin();
	for (; it != serverinfo.port.content_list.end(); ++it) {
		std::cout << it->c_str() << std::endl;
		struct sockaddr_in server_addr = defineServerAddress(it);
		int serverSocket_fd = createServerSocket();
		bindServerSocket(serverSocket_fd, server_addr);
		listenIncomingConnections(serverSocket_fd);
		std::cout << "Initial server socket listening on port " << atoi(it->c_str()) << std::endl;
		_vecServerSockets.push_back(serverSocket_fd);
	}
}
