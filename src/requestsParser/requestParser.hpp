#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <cstddef>
#include <iostream>

class requestParser {
public:
  requestParser();
  ~requestParser();

  std::string getHeader();
  std::string setHeader();

  std::string getBody();
  std::string setBody();

private:
  std::string _method;
  std::string _uri;
  std::string _contentType;
  std::size_t _contentLength;

  std::string _body;
};

#endif
