#include "requestHandler.hpp"
#include <exception>
#include <string>
#include <unordered_map>

requestHandler::requestHandler() : _request(nullptr) {}

requestHandler::requestHandler(Request &req) : _request(&req) {}

requestHandler::~requestHandler(){};

requestHandler::requestHandler(const requestHandler &src){};

requestHandler &requestHandler::operator=(const requestHandler &rhs) {
  return *this;
};

void requestHandler::handleRequest(const Request &request) {
  try {
    if (request.get_requestMethod() == "GET")
      handleGetRequest(request);
    if (request.get_requestMethod() == "POST")
      handlePOSTRequest(request);
    if (request.get_requestMethod() == "DELETE")
      handleDeleteRequest(request);
    else
      std::cout << "not a valid request type" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    throw e;
  }
  return;
}

std::string requestHandler::handleGetRequest(const Request &request) {
  std::string body = "<html>\
<head></head>\
<body><b>Hello World</b>\
<p>Hello World</p>Hello World</body>\
</html>";
  std::unordered_map<std::string, std::string> headers;

  headers["Content-Type"] = "text/plain";
  headers["Content-Length"] = std::to_string(body.length());

  return buildResponse(200, "Success", body, headers);
}

std::string requestHandler::handlePOSTRequest(const Request &request) {
  return "post";
};
std::string handleDeleteRequest(const Request &request) { return "delete"; };

std::string requestHandler::buildResponse(
    int status, const std::string &message, const std::string &body,
    const std::unordered_map<std::string, std::string> &headers) {
  return "test";
}
