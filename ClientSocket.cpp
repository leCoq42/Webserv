#include "ClientSocket.hpp"

ClientSocket::ClientSocket(){
}

ClientSocket::~ClientSocket() {
    for (size_t i = 0; i < _connectedClientSockets.size(); ++i) {
        int client_fd = _connectedClientSockets[i];
        close(client_fd);
    }
}

void	ClientSocket::addClientSocketToFds(int client_fd) {
	pollfd current_pollfd;
	current_pollfd.fd = client_fd;
	current_pollfd.events = POLLIN; // Monitoring for input events
	_polledDescriptors.push_back(current_pollfd);
}

void	ClientSocket::handleIncomingConnection(int serverSocket_fd) {
	int client_fd = accept(serverSocket_fd, nullptr, nullptr);
	if (client_fd == -1) {
		std::cerr << "Error accepting connection" << std::endl;
		return;
	}
	std::cout << "Accepted new connection on descriptor " << client_fd << std::endl;
	_connectedClientSockets.push_back(client_fd);
}

void	ClientSocket::handleInputEvent(int index, int serverSocket_fd) {
	if (_polledDescriptors[index].fd == serverSocket_fd) {
		// Incoming connection on server socket
		handleIncomingConnection(serverSocket_fd);
	} else {
		std::cout << "Input available on descriptor " << _polledDescriptors[index].fd << std::endl;
		// Handle input event
	}
}

void	ClientSocket::startPolling(int serverSocket_fd) {
	while (true) {
		_polledDescriptors.clear(); // Clear the vector before each iteration
		for (int i = 0; i < _connectedClientSockets.size(); i++) {
			addClientSocketToFds(_connectedClientSockets[i]);
		}
		int ret = poll(_polledDescriptors.data(), _polledDescriptors.size(), 5000);
		if (ret > 0) {
			for (size_t i = 0; i < _polledDescriptors.size(); i++) {
				if (_polledDescriptors[i].revents & POLLIN) {
					handleInputEvent(i, serverSocket_fd);
				}
				// Check for other events if needed (e.g., POLLOUT)
			}
		} else if (ret == 0) {
			std::cout << "Poll timeout" << std::endl;
			// Optionally continue polling or break out of the loop
		} else {
			std::cerr << "Poll error: " << strerror(errno) << std::endl;
			// Handle specific error cases or exit the loop
		}
	}
}
