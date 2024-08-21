#include "log.hpp"

Log::Log() : _logCount(0) 
{
    createPath();
    createLogFile();
}

Log::~Log() 
{
    if (_logFile.is_open())
        _logFile.close();
}

void Log::createPath() 
{	
	std::filesystem::path logFilePath(PATH_LOGFILE);
    if (!std::filesystem::exists(logFilePath)) {
        std::filesystem::path folderPath(logFilePath.parent_path());
        if (!std::filesystem::exists(folderPath)) {
            std::filesystem::create_directory(folderPath);
        }
    }
}

void Log::createLogFile() 
{
    _logFile.open(PATH_LOGFILE, std::ios_base::app);
    if (!_logFile) 
        std::cerr << "Error creating or opening logfile: " << PATH_LOGFILE << std::endl;
}

void Log::swapLogs() 
{
	const char* pathNewLogFile = "./logDir/newLogFile.log";
	std::ifstream oldLogFile(PATH_LOGFILE);
	if (!oldLogFile) {
		std::cerr << "Error opening old log file" << std::endl;
		return;
	}

	std::ofstream newLogFile(pathNewLogFile);
	if (!newLogFile) {
		std::cerr << "Error creating new log file" << std::endl;
		return;
	}

	std::deque<std::string> lastLines;
	std::string line;
	while (std::getline(oldLogFile, line)) {
		if (lastLines.size() >= AMOUNT_LINES_APPEND) {
			lastLines.pop_front();
		}
		lastLines.push_back(line);
	}
	oldLogFile.close();

	for (const auto& logLine : lastLines) {
		newLogFile << logLine << std::endl;
	}
	newLogFile.close();
	_logFile.close();
	if (std::remove(PATH_LOGFILE) != 0) {
		std::cerr << "Error deleting old logfile" << std::endl;
		return;
	}
	std::rename(pathNewLogFile, PATH_LOGFILE);
	createLogFile();
}

void Log::manageLogSize()
{
	_logCount += 1;
	if (_logCount > MAX_LOG_SIZE) {
		swapLogs();
		_logCount = 0;
	}
}

std::string Log::getTimeStamp()
{
	std::time_t now = std::time(nullptr);
	std::vector<char> buff(100);
	size_t len = std::strftime(&buff[0], buff.size(), "%Y/%m/%d %H:%M:%S", std::localtime(&now));
	return std::string(buff.begin(), buff.begin() + len);
}


void Log::logAdd(const std::string &message) 
{
	manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [info]  " << message << std::endl;
		_logFile.flush();
	}
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file" << std::endl;
	}
}

void Log::logError(const std::string &message)
{
	// manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [error] " << message << std::endl;
		_logFile.flush();
	}
	else
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file" << std::endl;
}

void Log::logClientError(const std::string &message, char *clientIP, int clientFD) 
{
	// manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [error] " << "Client IP " << clientIP << " "
						 << message << " on socket " << clientFD << std::endl;
		_logFile.flush();
	}
	else {
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file" << std::endl;
	}
}

void Log::logClientConnection(const std::string &message, std::string clientIP, int clientFD)
{
	// manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [info]  " << "Client IP " << clientIP << " "
						 << message << " on socket " << clientFD << std::endl;
		_logFile.flush();
	}
	else
		std::cerr << getTimeStamp() << " [error] Unable to write to access log file" << std::endl;
}

void Log::logServerError(const std::string &message,
												 const std::string &serverName, int port)
{
	// manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [error] " << message << " on server "
						 << serverName << "on port " << port << std::endl;
		_logFile.flush();
	}
	else
		std::cerr << getTimeStamp() << " [error] Unable to write to error log file" << std::endl;
}

void Log::logServerConnection(const std::string &message, const std::string &serverName, int socket, int port)
{
	// manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [info]  " << message << " " << serverName
						 << "on socket " << socket << " listening on port " << port << std::endl;
	_logFile.flush();
	}
	else
		std::cerr << getTimeStamp() << " [error] Unable to write to access log file" << std::endl;
}

void Log::logResponse(int status, const std::string &message) 
{
	// manageLogSize();
	if (_logFile.is_open()) {
		_logFile << getTimeStamp() << " [response] " << " Statuscode " << status
						 << "  " << message << " has been send to client" << std::endl;
		_logFile.flush();
	}
	else
		std::cerr << getTimeStamp() << " [error] Unable to write to access log file" << std::endl;
}
