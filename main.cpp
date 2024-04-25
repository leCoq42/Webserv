#include "ServerSocket.hpp"
#include "ClientSocket.hpp"

#define MAX_BUFFER_SIZE 1024

void	sendStuff(int client_fd) {
	char msg[] = "Whassaaaaabi paaapi!";
	int len = strlen(msg);
	int bytes_sent = send(client_fd, msg, len, 0);
	if (bytes_sent == -1) {
		std::cout << "Error send()" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void receiveStuff(int clientSocket_fd) {
    char buffer[MAX_BUFFER_SIZE];
    int bytes_received;

    bytes_received = recv(clientSocket_fd, buffer, MAX_BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        std::cerr << "Error in recv: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    } else if (bytes_received == 0) {
        std::cout << "Connection closed by remote side" << std::endl;
    } else {
        buffer[bytes_received] = '\0';
        std::cout << "Received " << bytes_received << " bytes: " << buffer << std::endl;
    }
}

int	acceptIncomingConnection(int serverSocket_fd){
	struct sockaddr_storage	client_addr;

	socklen_t	addr_size = sizeof(client_addr);
	int client_fd = accept(serverSocket_fd, (struct sockaddr *)&client_addr, &addr_size);
	return (client_fd);
}

int main() {
	ServerSocket SS;
	ClientSocket CS;

	// Creating server socket, binds it to port and listens for connections.
	struct sockaddr_in server_addr = SS.defineServerAddress();
	int serverSocket_fd = SS.createServerSocket();
	SS.bindServerSocket(serverSocket_fd, server_addr);
	SS.listenIncomingConnections(serverSocket_fd);
	std::cout << "Initial server socket listening on port 8080..." << std::endl;

	// Accepting incoming connections.
	int client_fd = acceptIncomingConnection(serverSocket_fd);

	// Send stuff
	sendStuff(client_fd);

	// Receive stuff
	receiveStuff(client_fd);

	// CS.startPolling(serverSocket_fd);

	close(serverSocket_fd);
	return (0);
}