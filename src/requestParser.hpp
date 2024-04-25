#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <iostream>
#include <map>

class requestParser {
public:
  requestParser();
  requestParser(const std::string &str);
  requestParser(const requestParser &src);
  requestParser &operator=(const requestParser &rhs);

  ~requestParser();

  bool parseRequest();
  bool checkRequestValidity();

  std::string get_rawRequest();
  void set_rawRequest(std::string &str);

  std::string get_requestMethod();
  void set_requestMethod(std::string &str);

  std::string get_uri();
  void set_uri(std::string &str);

  std::string get_htmlVersion();
  void set_htmlVersion(std::string &str);

  std::string get_body();
  void set_body(std::string &str);

  std::map<std::string, std::string> get_headers();

private:
  std::string _rawRequest;
  std::string _requestMethod;
  std::string _uri;
  std::string _htmlVersion;
  std::map<std ::string, std ::string> _headers;
  std::string _body;
};
#endif
