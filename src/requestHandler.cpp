#include "requestHandler.hpp"
#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

requestHandler::requestHandler() : _request(nullptr) {}

requestHandler::requestHandler(Request &req) : _request(&req) {}

requestHandler::~requestHandler(){};

requestHandler::requestHandler(const requestHandler &src){};

requestHandler &requestHandler::operator=(const requestHandler &rhs) {
  return *this;
};

std::string requestHandler::handleRequest(const Request &request) {
  try {
    if (request.get_requestMethod() == "GET")
      return handleGetRequest(request);
    if (request.get_requestMethod() == "POST")
      return handlePostRequest(request);
    if (request.get_requestMethod() == "DELETE")
      return handleDeleteRequest(request);
    else
      return buildResponse(405, "Method Not Allowed", "");
  } catch (const std::exception &e) {
    return buildResponse(500, "Internal Server Error", "");
  }
}

std::string requestHandler::handleGetRequest(const Request &request) {
  std::string resourcePath = request.get_uri();

  if (!resourcePath.empty() && resourcePath[0] == '/')
    resourcePath = resourcePath.substr(1);

  if (resourcePath.empty())
    resourcePath = "index.html";

  std::string contentType;
  std::size_t extensionPos = resourcePath.find_last_of('.');
  if (extensionPos != std::string::npos) {
    std::string extension = resourcePath.substr(extensionPos + 1);
    if (extension == "html" || extension == "htm")
      contentType = "text/html";
    else if (extension == "css")
      contentType = "text/css";
    else if (extension == "js")
      contentType = "application/javascript";
    else if (extension == "jpg" || extension == "jpeg")
      contentType = "image/jpeg";
    else if (extension == "png")
      contentType = "image/png";
    else
      contentType = "text/plain";
  }

  std::string path = "server/root/" + resourcePath;
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return buildResponse(404, "Not Found", "");

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string body = buffer.str();

  std::unordered_map<std::string, std::string> headers;
  headers["Content-Type"] = contentType;
  headers["Content-Length"] = std::to_string(body.length());

  return buildResponse(200, "OK", body, headers);
}

std::string requestHandler::handlePostRequest(const Request &request) {
  std::string requestBody = request.get_body();
  std::istringstream requestStream(requestBody);
  std::string line;
  std::unordered_map<std::string, std::string> formData;

  while (std::getline(requestStream, line)) {
    std::size_t seperatorPos = line.find('=');
    if (seperatorPos != std::string::npos) {
      std::string key = line.substr(0, seperatorPos);
      std::string value = line.substr(seperatorPos + 1);
      formData[key] = value;
    }
  }

  std::string response;

  std::unordered_map<std::string, std::string> headers;
  headers["Content-Type"] = "text/plain";
  headers["Content-Length"] = std::to_string(response.length());

  return buildResponse(200, "OK", response, headers);
};

std::string handleDeleteRequest(const Request &request) { return "delete"; };

std::string requestHandler::buildResponse(
    int status, const std::string &message, const std::string &body,
    const std::unordered_map<std::string, std::string> &headers) {
  std::stringstream response;
  response << "HTTP/1.1 " << status << " " << message << "\r\n";

  for (const auto &header : headers) {
    response << header.first << ": " << header.second << "/r/n";
  }

  if (!body.empty()) {
    if (headers.count("Content-Length") == 0) {
      response << "Content-Length: " << body.length() << "\r\n";
    }
    response << "\r\n" << body;
  } else {
    response << "\r\n";
  }

  return response.str();
}
