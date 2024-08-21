#include "signals.hpp"

volatile sig_atomic_t globalSignalReceived = 0;


void sigIntHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Interrupt signal received." << std::endl;
	globalSignalReceived = 1;
}

void sigQuitHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Quit signal received." << std::endl;
	globalSignalReceived = 1;
}

void initSignals()
{
	signal(SIGINT, sigIntHandler);
	signal(SIGQUIT, sigQuitHandler);
}
