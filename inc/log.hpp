#pragma once

#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

class Log {
public:
  Log();
  ~Log();

  // TODO:copy constructor and operator= overload

  bool addLogFile(const std::string &fileName);
  void logError(const std::string &message);
  void logClientError(const std::string &message, char *clientIP, int clientFD);
  void logServerError(const std::string &message, const std::string &serverName,
                      int port);
  void logClientConnection(const std::string &message, std::string clientIP,
                           int clientFD);
  void logServerConnection(const std::string &message,
                           const std::string &serverName, int socket, int port);
  void logResponse(int status, const std::string &message);

private:
  std::string getTimeStamp();
  std::ofstream _logFile;
};
