#include "log.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

// const long MAX_LOG_SIZE = 10 * 1024 * 1024;
#define PATH_LOGFILE "./logDir/logfile.log"

Log::Log() {
	addLogFile(PATH_LOGFILE);

	_logFile.open(PATH_LOGFILE, std::ios_base::app);
	if (!_logFile.is_open()) {
		std::cerr << getTimeStamp() << " [error] Unable to open access.log file" << std::endl;
	}
}

Log::~Log() {
	if (_logFile.is_open())
		_logFile.close();
}

void Log::addLogFile(const std::string &fileName) {
	std::filesystem::path logFilePath(fileName);
	if (!std::filesystem::exists(logFilePath))
	{
		std::filesystem::path folderPath(logFilePath.parent_path());
		if (!std::filesystem::exists(folderPath)) {
			std::filesystem::create_directory(folderPath);
		}
		std::ofstream file(logFilePath);
		if (!file) {
			std::cerr << "[error] creating logfile: " << logFilePath.string() << std::endl;
			return;
		}
		file.close(); // not necessary?
	}
}

long Log::getFileSize() {
	std::ifstream file(PATH_LOGFILE, std::ios::binary | std::ios::ate);
	return file.tellg();
}

std::string Log::getTimeStamp() {
	std::time_t now = std::time(nullptr);
	std::vector<char> buff(100);
	size_t len = std::strftime(&buff[0], buff.size(), "%Y/%m/%d %H:%M:%S", std::localtime(&now));
	return std::string(buff.begin(), buff.begin() + len);
}

// General log functions
void Log::logAdd(const std::string &message) {
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
void Log::logClientError(const std::string &message, char *clientIP,
												 int clientFD) {
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

void Log::logClientConnection(const std::string &message, std::string clientIP,
															int clientFD) {
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
