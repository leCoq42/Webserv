#include "Request.hpp"
#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

Request::Request() : _rawRequest("") {}

Request::Request(const std::string &rawStr) : _rawRequest(rawStr) {}

Request::~Request() {}

Request::Request(const Request &src)
    : _rawRequest(src._rawRequest), _requestMethod(src._requestMethod),
      _uri(src._uri), _htmlVersion(src._htmlVersion), _headers(src._headers),
      _body(src._body) {}

Request &Request::operator=(const Request &rhs) {
  Request temp(rhs);
  temp.swap(*this);
  return *this;
}

void Request::swap(Request &lhs) {
  std::swap(_requestMethod, lhs._requestMethod);
  std::swap(_uri, lhs._uri);
  std::swap(_htmlVersion, lhs._htmlVersion);
  std::swap(_headers, lhs._headers);
  std::swap(_body, lhs._body);
}

bool Request::parseRequest() {
  std::string line;
  std::string headerKey;
  std::string headerValue;

  if (_rawRequest.empty())
    return false;

  std::istringstream requestStream(_rawRequest);
  std::getline(requestStream, line);

  if (!parseRequestLine(line))
    return false;
  if (!Request::parseRequestHeaders(requestStream))
    return false;
  if (!Request::parseRequestBody(_rawRequest))
    return false;
  checkRequestValidity();

  return true;
}

bool Request::parseRequestLine(const std::string &line) {
  std::istringstream lineStream(line);
  if (!(lineStream >> _requestMethod >> _uri >> _htmlVersion))
    return false;
  return true;
}

bool Request::parseRequestHeaders(std::istringstream &requestStream) {
  std::string line;
  size_t pos;
  std::string headerKey;
  std::string headerValue;

  while (std::getline(requestStream, line) && line != "\r") {
    pos = line.find(":");
    if (pos != std::string::npos) {
      headerKey = line.substr(0, pos);
      headerValue = line.substr(pos + 2);
      _headers[headerKey] = headerValue;
    }
  }
  return true;
}

bool Request::parseRequestBody(const std::string &_rawRequest) {
  size_t body_start;
  size_t content_len;

  std::unordered_map<std::string, std::string>::iterator content_len_it =
      _headers.find("Content-Length");
  if (content_len_it == _headers.end())
    return false;

  try {
    content_len = std::stoul(content_len_it->second);
  } catch (const std::invalid_argument) {
    return false;
  } catch (const std::out_of_range) {
    return false;
  }

  body_start = _rawRequest.find("\r\n\r\n");
  if (body_start == std::string::npos) {
    return false;
  }
  _body = _rawRequest.substr(body_start + 4, content_len);
  return true;
}

// TODO: max length of GET request 2048 bytes?
bool Request::checkRequestValidity() {
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

const std::string &Request::get_rawRequest() const { return _rawRequest; }
const std::string &Request::get_requestMethod() const { return _requestMethod; }
const std::string &Request::get_uri() const { return _uri; }
const std::string &Request::get_body() const { return _body; }
const std::string &Request::get_htmlVersion() const { return _htmlVersion; }
const std::unordered_map<std::string, std::string> &
Request::get_headers() const {
  return _headers;
}
