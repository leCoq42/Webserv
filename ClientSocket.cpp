// #include "ClientSocket.hpp"

// ClientSocket::ClientSocket(){
// }

// ClientSocket::~ClientSocket() {
//     for (size_t i = 0; i < _connectedClientSockets.size(); ++i) {
//         int client_fd = _connectedClientSockets[i];
//         close(client_fd);
//     }
// }

// void	ClientSocket::addClientSocketToFds(int serverSocker_fd) {
// 	pollfd current_pollfd;
// 	current_pollfd.fd = serverSocker_fd;
// 	current_pollfd.events = POLLIN; // Monitoring for input events
// 	pfds.push_back(current_pollfd);
// 	//pfds is used for the purpose of managing and 
// 	// monitoring file descriptors for events using the poll system call
// }

// void	ClientSocket::handleIncomingConnection(int serverSocket_fd) {
// 	int client_fd = accept(serverSocket_fd, nullptr, nullptr);
// 	if (client_fd == -1) {
// 		std::cerr << "Error accepting connection" << std::endl;
// 		return;
// 	}
// 	std::cout << "Accepted new connection on descriptor " << client_fd << std::endl;
// 	_connectedClientSockets.push_back(client_fd);
// }

// void	ClientSocket::handleInputEvent(int index, int serverSocket_fd) {
// 	if (pfds[index].fd == serverSocket_fd) {
// 		// Incoming connection on server socket
// 		handleIncomingConnection(serverSocket_fd);
// 	} else {
// 		std::cout << "Input available on descriptor " << pfds[index].fd << std::endl;
// 		// Handle input event
// 	}
// }

// void	ClientSocket::startPolling(int serverSocket_fd) {
// 	while (true) {
// 		pfds.clear(); // Clear the vector before each iteration
// 		// for (int i = 0; i < _connectedClientSockets.size(); i++) {
// 		// 	std::cout << "added client socked fd" << _connectedClientSockets[i] << std::endl;
// 		// 	addClientSocketToFds(_connectedClientSockets[i]);
// 		// }
// 		addClientSocketToFds(serverSocket_fd);
// 		int poll_count = poll(pfds.data(), pfds.size(), 5000);
// 		if (poll_count > 0) {
// 			for (size_t i = 0; i < pfds.size(); i++) {
// 				if (pfds[i].revents & POLLIN) {
// 					std::cout << "looking to handle event" << std::endl;
// 					handleInputEvent(i, serverSocket_fd);
// 				}
// 				// Check for other events if needed (e.g., POLLOUT)
// 			}
// 		} else if (poll_count == 0) {
// 			std::cout << "Poll timeout" << std::endl;
// 			// Optionally continue polling or break out of the loop
// 		} else {
// 			std::cerr << "Poll error: " << strerror(errno) << std::endl;
// 			// Handle specific error cases or exit the loop
// 		}
// 	}
// }


#include "ClientSocket.hpp"

ClientSocket::ClientSocket(){
}

ClientSocket::~ClientSocket() {
    for (size_t i = 0; i < _connectedClientSockets.size(); ++i) {
        int client_fd = _connectedClientSockets[i];
        close(client_fd);
    }
}

void ClientSocket::handleInputEvent(int index) {
    
	char buffer[1024];
    int client_fd = pfds[index].fd;

	const char* messageFromServer = "Input message to data: \n";
    ssize_t bytesSent = send(client_fd, messageFromServer, strlen(messageFromServer), 0);
    if (bytesSent == -1) {
        std::cerr << "Error sending data to client: " << strerror(errno) << std::endl;
        // Handle error
        return;
    }
    
    ssize_t bytesRead = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytesRead == -1) {
        std::cerr << "Error receiving data from client: " << strerror(errno) << std::endl;
        // Handle error
        return;
    } else if (bytesRead == 0) {
        std::cout << "Client disconnected" << std::endl;
        // Handle disconnection
        return;
    }

    // Process received data
    buffer[bytesRead] = '\0'; // Null-terminate the received data
	std::string bufferstr = buffer;
	std::cout << buffer << std::endl;
	std::cout << bufferstr << std::endl;
	if (bufferstr.find("exit")){
		exit(EXIT_SUCCESS);
	}
    std::cout << "Received data from client: " << buffer << std::endl;

    // Example: Echo back the received data
    // ssize_t bytesSent = send(client_fd, buffer, bytesRead, 0);
    // if (bytesSent == -1) {
    //     std::cerr << "Error sending data to client: " << strerror(errno) << std::endl;
    //     // Handle error
    //     return;
    // }
    
    // Optionally, you can also prompt for and send data from the server side.
    // For example, let's send a message from the server to the client.

}

// void	ClientSocket::addClientSocketToFds(int serverSocker_fd) {
// 	pollfd current_pollfd;
// 	current_pollfd.fd = serverSocker_fd;
// 	current_pollfd.events = POLLIN; // Monitoring for input events
// 	pfds.push_back(current_pollfd);
// 	//pfds is used for the purpose of managing and 
// 	// monitoring file descriptors for events using the poll system call
// }

void ClientSocket::addClientSocketToFds(int serverSocket_fd) {
    // Add server socket
    pollfd server_pollfd;
    server_pollfd.fd = serverSocket_fd;
    server_pollfd.events = POLLIN; // Monitoring for input events
    pfds.push_back(server_pollfd);

    // Add client sockets
    for (size_t i = 0; i < _connectedClientSockets.size(); ++i) {
        pollfd client_pollfd;
        client_pollfd.fd = _connectedClientSockets[i];
        client_pollfd.events = POLLIN; // Monitoring for input events
        pfds.push_back(client_pollfd);
    }
}

void	ClientSocket::handleIncomingConnection(int index, int serverSocket_fd) {
		struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

		int client_fd = accept(serverSocket_fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
		if (client_fd == -1) {
			std::cerr << "Error accepting connection" << std::endl;
			return;
		}
		std::cout << "Accepted new connection on server socket " << serverSocket_fd << " from client socket " << client_fd << std::endl;
		_connectedClientSockets.push_back(client_fd);
	const char* messageFromServer = "Input message to data: \n";
    ssize_t bytesSent = send(client_fd, messageFromServer, strlen(messageFromServer), 0);
    if (bytesSent == -1) {
        std::cerr << "Error sending data to client: " << strerror(errno) << std::endl;
        // Handle error
        return;
    }
}

void	ClientSocket::startPolling(int serverSocket_fd) {
	while (true) {
		pfds.clear(); // Clear the vector before each iteration
		// for (int i = 0; i < _connectedClientSockets.size(); i++) {
		// 	std::cout << "added client socked fd" << _connectedClientSockets[i] << std::endl;
		// 	addClientSocketToFds(_connectedClientSockets[i]);
		// }
		addClientSocketToFds(serverSocket_fd);
		int poll_count = poll(pfds.data(), pfds.size(), 5000);
        if (poll_count > 0) {
            for (size_t i = 0; i < pfds.size(); i++) {
                if (pfds[i].revents & POLLIN) {
                    if (pfds[i].fd == serverSocket_fd) {
                        // Incoming connection on server socket
                        handleIncomingConnection(i, serverSocket_fd);
                    } else {
                        // Input available on client socket
                        std::cout << "Input available on descriptor " << pfds[i].fd << std::endl;
                        handleInputEvent(i);
                    }
                }
                // Check for other events if needed (e.g., POLLOUT, POLLHUP)
                if (pfds[i].revents & (POLLHUP | POLLERR)) {
                    // Handle disconnection or error on the socket
                    std::cerr << "Error or disconnection on descriptor " << pfds[i].fd << std::endl;
                    // Close the socket and remove it from pfds if necessary
                }
            }
        } else if (poll_count == 0) {
            // std::cout << "Poll timeout" << std::endl;
            // Optionally continue polling or break out of the loop
        } else {
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            // Handle specific error cases or exit the loop
        }
    }
}