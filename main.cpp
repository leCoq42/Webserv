#include "Poll.hpp"
#include "Socket.hpp"

int createSocket() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Error creating socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

struct sockaddr_in defineServerAddress() {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;             // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;     // Accept connections from any IP address
    server_addr.sin_port = htons(8080);           // Port 8080
    return server_addr;
}

void bindServerSocket(int server_fd, struct sockaddr_in &server_addr) {
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}

void listenIncomingConnections(int server_fd) {
    if (listen(server_fd, 5) == -1) { // adjust the value (5) based on the performance and scalability needs.
        std::cerr << "Error listening for connection on server socket" << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}

void closeClientSockets(const std::vector<int> &vec_clients_fds) {
    for (int client_fd : vec_clients_fds) {
        close(client_fd);
    }
}

void startPolling(int server_fd, std::vector<int> &vec_client_fds) {
    while (true) {
        std::vector<pollfd> vec_pollfd;
	for (int i = 0; i < vec_client_fds.size(); i++) {
   		 int client_fd = vec_client_fds[i];
   		 pollfd current_pollfd;
   		 current_pollfd.fd = client_fd;
   		 current_pollfd.events = POLLIN; // Monitoring for input events
   		 vec_pollfd.push_back(current_pollfd);
}

    int ret = poll(vec_pollfd.data(), vec_pollfd.size(), 5000);
    if (ret > 0) {
        for (size_t i = 0; i < vec_pollfd.size(); i++) {
            if (vec_pollfd[i].revents & POLLIN) {
                if (vec_pollfd[i].fd == server_fd) {
                    // Incoming connection on server socket
                    int client_fd = accept(server_fd, nullptr, nullptr);
                    if (client_fd == -1) {
                        std::cerr << "Error accepting connection" << std::endl;
                        continue;
                    }
                    std::cout << "Accepted new connection on descriptor " << client_fd << std::endl;
                    vec_client_fds.push_back(client_fd);
                } else {
                    std::cout << "Input available on descriptor " << vec_pollfd[i].fd << std::endl;
                    // Handle input event
                }
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

int main() {
    std::vector<int> vec_clients_fds; // Pool of client sockets

    int server_fd = createSocket();
    struct sockaddr_in server_addr = defineServerAddress();
    bindServerSocket(server_fd, server_addr);
    listenIncomingConnections(server_fd);

    std::cout << "Initial server socket listening on port 8080..." << std::endl;

    startPolling(server_fd, vec_clients_fds);

    close(server_fd);
    closeClientSockets(vec_clients_fds);
    return 0;
}