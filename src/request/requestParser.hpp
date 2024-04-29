#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <iostream>
#include <unordered_map>

class Request {
public:
  Request();
  Request(const std::string &str);
  Request(const Request &src);
  Request &operator=(const Request &rhs);

  ~Request();

  bool parseRequest();
  bool parseRequestLine(const std::string &line);
  bool parseRequestHeaders(std::istringstream &requestStream);
  bool parseRequestBody(const std::string &_rawRequest);
  bool checkRequestValidity();

  const std::string &get_rawRequest() const;

  const std::string &get_requestMethod() const;

  const std::string &get_uri() const;

  const std::string &get_htmlVersion() const;

  const std::string &get_body() const;

  const std::unordered_map<std::string, std::string> &get_headers() const;

private:
  const std::string _rawRequest;
  std::string _requestMethod;
  std::string _uri;
  std::string _htmlVersion;
  std::unordered_map<std::string, std::string> _headers;
  std::string _body;
};
#endif
