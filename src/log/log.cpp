#include "log.hpp"

Log::Log() {
	_logFile.open("./log/log.log", std::ios_base::app);
	if (!_logFile.is_open()) {
		std::cerr << getTimeStamp() << " [error] Unable to open access.log file" << std::endl;
	}
}

Log::~Log() {
	if (_logFile.is_open())
		_logFile.close();
}

std::string Log::getTimeStamp() {
	std::time_t now = std::time(nullptr);
	char buff[100];
	std::strftime(buff, sizeof(buff), "%Y/%m/%d %H:%M:%S", std::localtime(&now));
	return std::string(buff);
}


// General log functions
void	Log::logError(const std::string& message) {
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [error] " << message << std::endl;
		_logFile.flush();
	} 
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file" << std::endl;
	}
}

// Client log functions
void	Log::logClientError(const std::string& message, char* clientIP, int clientFD) {
	if (_logFile.is_open()) {
			_logFile << getTimeStamp() << " [error] " << clientIP << " " << message << " on socket " << clientFD << std::endl;
		_logFile.flush();
	} 
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file" << std::endl;
	}
}

void	Log::logClientConnection(const std::string& message, std::string clientIP, int clientFD) {
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [info] " << "Client IP "<< clientIP << " " << message << " on socket " << clientFD << std::endl;
		_logFile.flush();
	} 
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to access log file" << std::endl;
	}
}


// Server log functions
void	Log::logServerError(const std::string& message, const std::string& serverName, int port) {
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [error] " << message << " on server " << serverName 
					  << ", on port " << port << std::endl;
		_logFile.flush();
	} 
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file" << std::endl;
	}
}

void	Log::logServerConnection(const std::string& message, const std::string& serverName, int socket, int port) {
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [info] " << message << " " << serverName << " on socket " << socket 
					   << " is now listening on port " << port << std::endl;
		_logFile.flush();
	} 
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to access log file" << std::endl;
	}
}