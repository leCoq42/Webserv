#include "../inc/Webserv.hpp"

int	main(void){
	ServerSocket SS;
	ClientSocket CS;

	int serverSocket_fd = SS.setUpServerSocket();
	CS.startPolling(serverSocket_fd);
	close(serverSocket_fd);

	return(0);
}
