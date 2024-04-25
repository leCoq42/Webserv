#include "requestParser.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>

requestParser::requestParser() {}

requestParser::requestParser(const std::string &rawStr) : _rawRequest(rawStr) {}

requestParser::~requestParser() {}

bool requestParser::parseRequest() {
  std::string line;
  std::string headerKey;
  std::string headerValue;
  size_t pos;

  if (_rawRequest.empty())
    return 1;

  std::istringstream requestStream(_rawRequest);
  std::getline(requestStream, line);

  std::istringstream lineStream(line);
  if (!(lineStream >> _requestMethod >> _uri >> _htmlVersion))
    return 1;

  while (std::getline(requestStream, line) && line != "\r") {
    pos = line.find(":");
    if (pos != std::string::npos) {
      headerKey = line.substr(0, pos);
      headerValue = line.substr(pos + 2);
      _headers[headerKey] = headerValue;
    }
  }

  std::ostringstream oss;
  oss << requestStream.rdbuf();
  _body = oss.str();

  return 0;
}

// TODO: max length of GET request 2048 bytes?
bool requestParser::checkRequestValidity() {
  if (_requestMethod.empty() || _htmlVersion.empty() || _uri.empty())
    return false;
  if (_requestMethod != "GET" && _requestMethod != "POST" &&
      _requestMethod != "DELETE")
    return false;
  if (_htmlVersion != "HTTP/1.0" && _htmlVersion != "HTTP/1.1" &&
      _htmlVersion != "HTTP/2.0")
    return false;
  if (_htmlVersion != "HTTP/1.0" && _headers.find("Host") == _headers.end())
    return false;

  return true;
}

std::string requestParser::get_rawRequest() { return _rawRequest; }
void requestParser::set_rawRequest(std::string &str) { _rawRequest = str; }

std::string requestParser::get_requestMethod() { return _requestMethod; }
void requestParser::set_requestMethod(std::string &str) {
  _requestMethod = str;
}

std::string requestParser::get_uri() { return _uri; }
void requestParser::set_uri(std::string &str) { _uri = str; }

std::string requestParser::get_body() { return _body; }
void requestParser::set_body(std::string &str) { _body = str; }

std::string requestParser::get_htmlVersion() { return _htmlVersion; }
void requestParser::set_htmlVersion(std::string &str) { _htmlVersion = str; }

std::map<std::string, std::string> requestParser::get_headers() {
  return _headers;
}
