#include "ServerSocket.hpp"

ServerSocket::ServerSocket(){
}

ServerSocket::~ServerSocket() {
}

struct sockaddr_in	ServerSocket::defineServerAddress() {
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;     		  // IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY;     // Accept connections from any IP address
	server_addr.sin_port = htons(8080);      	  // Port 8080
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

int	ServerSocket::setUpServerSocket() {
	struct sockaddr_in server_addr = defineServerAddress();
	int serverSocket_fd = createServerSocket();
	bindServerSocket(serverSocket_fd, server_addr);
	listenIncomingConnections(serverSocket_fd);
	std::cout << "Initial server socket listening on port 8080..." << std::endl;
	return (serverSocket_fd);
}
