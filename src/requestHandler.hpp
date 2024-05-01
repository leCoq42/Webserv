#pragma once

#include "request/Request.hpp"
#include <iostream>
#include <unordered_map>

class requestHandler {
public:
  requestHandler();
  requestHandler(Request &req);
  ~requestHandler();

  requestHandler(const requestHandler &src);
  requestHandler &operator=(const requestHandler &rhs);
  void swap(requestHandler &tmp);

  void handleRequest(const Request &request);
  std::string sendResponse(const Request &request);

private:
  Request *_request;
  std::string handleGetRequest(const Request &request);
  std::string handlePOSTRequest(const Request &request);
  std::string handleDeleteRequest(const Request &request);
  std::string
  buildResponse(int status, const std::string &message, const std::string &body,
                const std::unordered_map<std::string, std::string> &headers);
};
