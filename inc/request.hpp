#pragma once

#include <cstddef>
#include <iostream>
#include <unordered_map>
#include <vector>

// doesn't seem to be the standard but it is an security issue not to check
// this, https://www.htmlhelp.com/faq/cgifaq.2.html
// https://datatracker.ietf.org/doc/html/rfc3875#section-4
//  std::string	specifiedCgiEnv[] = {}

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
  const std::string get_referer() const;
  const std::string get_contentType() const;
  size_t get_contentLen() const;
  const std::string get_boundary() const;
  const std::vector<std::string> &get_requestArgs() const;
  const std::string &get_htmlVersion() const;
  const std::string &get_connection() const;
  const std::string &get_body() const;
  const std::unordered_map<std::string, std::string> &get_headers() const;
  void		set_bufferFile(std::string buffer_file); //added
  const std::string &get_bufferFile() const;
  void		keepAlive(bool keepAlive); //added
	void	set_startContentLength(size_t content_length);
	const size_t	&get_startContentLength(void) const;
  const bool &get_keepAlive() const;
  const bool &get_validity() const;

  void printRequest() const;

private:
  const std::string _rawRequest;
  std::string _requestMethod;
  std::string _uri;
  std::string _htmlVersion;
  std::vector<std::string> _requestArgs;
  std::unordered_map<std::string, std::string> _headers;
  bool _keepAlive;
  std::string _body;
  bool _isValid;
  std::unordered_map<std::string, std::string> _cgiEnv;
  std::string	_bufferFile;//added
  size_t		_startContentLength;

  void parseRequest();
  std::vector<std::string> parse_requestArgs(const std::string uri);
  bool parseRequestLine(const std::string &line);
  bool parseRequestHeaders(std::istringstream &requestStream);
  bool parseRequestBody(const std::string &_rawRequest);
  void extractCgiEnv(void);
};
