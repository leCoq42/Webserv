#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <cstring>

class Log {
public:
    Log();
    ~Log();

    void	logError(const std::string& message);
    void    logClientError(const std::string& message, char* clientIP, int clientFD);
    void    logServerError(const std::string& message, const std::string& serverName, int port);
    void    logClientConnection(const std::string& message, std::string clientIP, int clientFD);
    void    logServerConnection(const std::string& message, const std::string& serverName, int socket, int port);

private:
    std::string getTimeStamp();
    std::ofstream _logFile;
};