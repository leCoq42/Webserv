#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>

class	Log {
	public:
		Log();
		~Log();
		
		void	logError(const std::string& message, const std::string& client_ip, const std::string& request);
		// void	logAccessClient(const std::string& client_ip, const std::string& request, int status, int size, const std::string& referrer, const std::string& user_agent);
		void	logServerConnection(const std::string& serverName, int socket, int port);

	private:	
		std::string	getTimeStamp();
		std::ofstream _accessLogFile;
		std::ofstream _errorLogFile;
};