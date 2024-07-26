#include "log.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>



Log::Log() : _logCount(0), _buffer("") {
	checkPath(PATH_LOGFILE);
	createLogFile();
}

Log::~Log() {
	if (_logFile.is_open())
		_logFile.close();
}

void Log::checkPath(const std::string &path_Logfile) {
	std::filesystem::path logFilePath(path_Logfile);
	if (!std::filesystem::exists(logFilePath))
	{
		std::filesystem::path folderPath(logFilePath.parent_path());
		if (!std::filesystem::exists(folderPath)) {
			std::filesystem::create_directory(folderPath);
		}

	}
}

void	log::createLogFile() {
	std::ofstream file(PATH_LOGFILE);
		if (!file) {
			std::cerr << "[error] creating logfile: " << PATH_LOGFILE << std::endl;
			return;
		}
	_logFile.open(PATH_LOGFILE, std::ios_base::app);
	if (!_logFile.is_open()) {
		std::cerr << getTimeStamp() << " [error] Unable to open access.log file" << std::endl;
	}
}

void	Log::swapLogs() {
		const char* pathTempFile = "./logDir/tempFile.log";
		std::ofstream tempFile(pathTempFile);
		if (!tempFile) {
			std::cerr << "[error] creating tempFile: " << pathTempFile << std::endl;
			return;
		}
		std::string line;
		while(std::getline, line) {
			tempFile << line << std::endl;
		}
		_logFile.close();
		if (remove(PATH_LOGFILE) == 0) 
			std::cout << "File deleted successfully" << std::endl;
		createLogFile();
		while(getline(tempFile, line)) {
			_logFile << line << std::endl;
		}
		tempFile.close();
}

void	Log::manageLogSize() {
	_logCount += 1;
	if (_logCount > MAX_LOG_SIZE) {
		swapLogs();
		_logCount = 0;
	}
}


std::string Log::getTimeStamp() {
	std::time_t now = std::time(nullptr);
	std::vector<char> buff(100);
	size_t len = std::strftime(&buff[0], buff.size(), "%Y/%m/%d %H:%M:%S", std::localtime(&now));
	return std::string(buff.begin(), buff.begin() + len);
}

// General log functions
void Log::logAdd(const std::string &message) {
	manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [info]  " << message << std::endl;
		_logFile.flush();
	}
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file"
							<< std::endl;
	}
}

void Log::logError(const std::string &message) {
	manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [error] " << message << std::endl;
		_logFile.flush();
	}
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file"
							<< std::endl;
	}
}

// Client connection log functions
void Log::logClientError(const std::string &message, char *clientIP, int clientFD) {
	manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [error] " << "Client IP " << clientIP << " "
						 << message << " on socket " << clientFD << std::endl;
		_logFile.flush();
	} 
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file"
							<< std::endl;
	}
}

void Log::logClientConnection(const std::string &message, std::string clientIP, int clientFD) {
	manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [info]  " << "Client IP " << clientIP << " "
						 << message << " on socket " << clientFD << std::endl;
		_logFile.flush();
	} 
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to access log file"
							<< std::endl;
	}
}

// Server connection log functions
void Log::logServerError(const std::string &message,
												 const std::string &serverName, int port) {
	manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [error] " << message << " on server "
						 << serverName << "on port " << port << std::endl;
		_logFile.flush();
	} 
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file"
							<< std::endl;
	}
}

void Log::logServerConnection(const std::string &message,
															const std::string &serverName, int socket,
															int port) {
	manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [info]  " << message << " " << serverName
						 << "on socket " << socket << " listening on port " << port
						 << std::endl;
		_logFile.flush();
	} 
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to access log file"
							<< std::endl;
	}
}

// Response log functions
void Log::logResponse(int status, const std::string &message) {
	manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [response] " << " Statuscode " << status
						 << "  " << message << " has been send to client" << std::endl;
		_logFile.flush();
	} 
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to access log file"
							<< std::endl;
	}
}
