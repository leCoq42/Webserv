#include "ServerSocket.hpp"
#include "ClientSocket.hpp"

int main() {
	ServerSocket SS;
	ClientSocket CS;

	struct sockaddr_in server_addr = SS.defineServerAddress();
	int serverSocket_fd = SS.createServerSocket();
	SS.bindServerSocket(serverSocket_fd, server_addr);
	SS.listenIncomingConnections(serverSocket_fd);

	std::cout << "Initial server socket listening on port 8080..." << std::endl;

	CS.startPolling(serverSocket_fd);

	close(serverSocket_fd);
	return (0);
}