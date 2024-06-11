#include "log.hpp"

Log::Log() {
	 _accessLogFile.open("./log/access.log", std::ios_base::app);
	if (!_accessLogFile.is_open()){
			std::cerr << "Error: Unable to open access.log file" << std::endl;
	}
	   _errorLogFile.open("./log/error.log", std::ios_base::app);
	if (!_errorLogFile.is_open()){
			std::cerr << "Error: Unable to open error.log file" << std::endl;
	}
}

Log::~Log() {
	if (_accessLogFile.is_open()) {
		_accessLogFile.close();
	}
	if (_errorLogFile.is_open()) {
		_errorLogFile.close();
	}
}

std::string Log::getTimeStamp() {
	std::time_t now = std::time(nullptr);
	char buff[100];
	std::strftime(buff, sizeof(buff), "%Y/%m/%d %H:%M:%S", std::localtime(&now));
	return (buff);
}

void Log::logAccess(const std::string& client_ip, const std::string& request, int status, int size, 
	const std::string& referrer, const std::string& user_agent) {
	if (_accessLogFile.is_open()) {
		_accessLogFile << client_ip << " - - [" << getTimeStamp() << "] \"" << request << "\" " << status << " " << size << " \"" << referrer << "\" \"" << user_agent << "\"\n";
		_accessLogFile.flush();
	}
}

void Log::logError(const std::string& message, const std::string& client_ip, const std::string& request) {
	if (_errorLogFile.is_open()) {
		_errorLogFile << getTimeStamp() << " [error] " << message << ", client: " << client_ip << ", request: \"" << request << "\"\n";
		_errorLogFile.flush();
	}
}
