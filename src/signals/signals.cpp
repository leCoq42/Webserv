#include "signals.hpp"

volatile sig_atomic_t globalSignalReceived = 0;


void sigHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Interrupt signal received." << std::endl;
	std::cout << "Webserv is closed." << std::endl;
	globalSignalReceived = 1;
}

void initSignals()
{
	signal(SIGINT, sigHandler);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
}
