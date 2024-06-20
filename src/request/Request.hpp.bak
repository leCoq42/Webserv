#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

class Request {
public:
  Request();
  Request(const std::string &str);
  Request(const Request &src);
  Request &operator=(const Request &rhs);
  void swap(Request &lhs);
  ~Request();

  bool checkRequestValidity() const;

  void print_Request();

  const std::string &get_rawRequest() const;
  const std::string &get_requestMethod() const;
  const std::string &get_uri() const;
  const std::vector<std::string> &get_requestArgs() const;
  const std::string &get_htmlVersion() const;
  const std::string &get_connection() const;
  const std::string &get_body() const;
  const std::unordered_map<std::string, std::string> &get_headers() const;
  const bool &get_keepAlive() const;
  const bool &get_validity() const;

  void printRequest() const;

  const std::string _rawRequest;
  std::string _requestMethod;
  std::string _uri;
  std::vector<std::string> _requestArgs;
  std::string _htmlVersion;
  std::unordered_map<std::string, std::string> _headers;
  bool _keepAlive;
  std::string _body;
  bool _isValid;

  void parseRequest();
  std::vector<std::string> parse_requestArgs(const std::string uri);
  bool parseRequestLine(const std::string &line);
  bool parseRequestHeaders(std::istringstream &requestStream);
  bool parseRequestBody(const std::string &_rawRequest);
private:
};
