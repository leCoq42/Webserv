#include "ServerSocket.hpp"
#include "ClientSocket.hpp"

int main() {
	ServerSocket SS;
	ClientSocket CS;

	int serverSocket_fd = SS.setUpServerSocket();
	CS.startPolling(serverSocket_fd);
	close(serverSocket_fd);
	return (0);
}