#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

class Request {
public:
  void parseRequest();
  bool parseRequestLine(const std::string &line);
  bool parseRequestHeaders(std::istringstream &requestStream);
  bool parseRequestBody(const std::string &_rawRequest);
  bool checkRequestValidity();

  std::string _rawRequest;
  std::string _requestMethod;
  std::string _uri;
  std::string _htmlVersion;
  std::unordered_map<std::string, std::string> _headers;
  std::string _body;
  bool _isValid;
};

void Request::parseRequest() {
  std::string line;

  if (_rawRequest.empty()) {
    _isValid = false;
    return;
  }

  std::istringstream requestStream(_rawRequest);
  std::getline(requestStream, line);

  if (!parseRequestLine(line) || !parseRequestHeaders(requestStream) ||
      !parseRequestBody(_rawRequest)) {
    _isValid = false;
    return;
  }
  _isValid = checkRequestValidity();
}

bool Request::parseRequestLine(const std::string &line) {
  std::istringstream lineStream(line);
  if (!(lineStream >> _requestMethod >> _uri >> _htmlVersion)) {
    std::cerr << "Failed to parse request line: " << line << std::endl;
    return false;
  }
  std::cout << "Request Method: " << _requestMethod << std::endl;
  std::cout << "URI: " << _uri << std::endl;
  std::cout << "HTML Version: " << _htmlVersion << std::endl;
  return true;
}

bool Request::parseRequestHeaders(std::istringstream &requestStream) {
  std::string line;
  size_t pos;

  while (std::getline(requestStream, line) && !line.empty() && line != "\r") {
    pos = line.find(":");
    if (pos != std::string::npos) {
      std::string headerKey = line.substr(0, pos);
      std::string headerValue = line.substr(pos + 1);
      if (!headerValue.empty() && headerValue[0] == ' ') {
        headerValue = headerValue.substr(1);
      }
      _headers[headerKey] = headerValue;
      std::cout << "Header: " << headerKey << " => " << headerValue
                << std::endl;
    }
  }
  return true;
}

bool Request::parseRequestBody(const std::string &_rawRequest) {
  size_t body_start;
  size_t content_len;

  auto content_len_it = _headers.find("Content-Length");
  if (content_len_it == _headers.end()) {
    std::cerr << "Content-Length header not found" << std::endl;
    return false;
  }

  try {
    content_len = std::stoul(content_len_it->second);
  } catch (const std::invalid_argument &e) {
    std::cerr << "Invalid Content-Length: " << content_len_it->second
              << std::endl;
    return false;
  } catch (const std::out_of_range &e) {
    std::cerr << "Content-Length out of range: " << content_len_it->second
              << std::endl;
    return false;
  }

  body_start = _rawRequest.find("\r\n\r\n");
  if (body_start == std::string::npos) {
    std::cerr << "Body start not found" << std::endl;
    return false;
  }
  _body = _rawRequest.substr(body_start + 4, content_len);
  std::cout << "Body: " << _body << std::endl;
  return true;
}

bool Request::checkRequestValidity() {
  // Example validity check
  return !_requestMethod.empty() && !_uri.empty() && !_htmlVersion.empty();
}

int main() {
  // Example rawRequest string
  std::string rawRequest =
      "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: "
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\nAccept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
      "*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, "
      "deflate\r\nConnection: keep-alive\r\n\r\n";

  Request req;
  req._rawRequest = rawRequest;
  req.parseRequest();

  std::cout << "Request method: " << req._requestMethod << std::endl;
  std::cout << "URI: " << req._uri << std::endl;
  std::cout << "HTML version: " << req._htmlVersion << std::endl;

  std::cout << "*** Headers ***" << std::endl;
  for (const auto &header : req._headers) {
    std::cout << header.first << ": " << header.second << std::endl;
  }

  std::cout << "*** Body ***" << std::endl;
  std::cout << "body: " << req._body << std::endl;

  std::cout << "Validity check: " << (req._isValid ? "Valid!" : "Invalid!")
            << std::endl;

  return 0;
}
