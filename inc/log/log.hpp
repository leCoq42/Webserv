#pragma once

#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <defines.hpp>
#include <fstream>
#include <deque>
#include <filesystem>
#include <vector>

#define MAX_LOG_SIZE 1000
#define AMOUNT_LINES_APPEND 100
#define PATH_LOGFILE "./logDir/logfile.log"

class Log {
private:
	static std::string	getTimeStamp();
	std::ofstream		_logFile;

public:
	Log();
	~Log();

	void	createPath();
	void	createLogFile();
	void	swapLogs();

	void	logError(const std::string &message);
	void	logClientError(const std::string &message, char *clientIP, int clientFD);
	void	logServerError(const std::string &message, const std::string &serverName,
							 int port);
	void	logClientConnection(const std::string &message, std::string clientIP, int clientFD);
	void	logServerConnection(const std::string &message,
								 const std::string &serverName, int socket, int port);
	void	logResponse(int status, const std::string &message);
	void	logAdd(const std::string &message);
	void	manageLogSize();
};
