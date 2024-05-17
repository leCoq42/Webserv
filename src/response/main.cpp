#include "requestParser.hpp"
#include <iostream>
#include <ostream>

#define GET_EXAMPLE                                                            \
  "GET /index.html HTTP/1.1\r\n\
Host: www.example.com\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n\
Accept-Language: en-US,en;q=0.5\r\n\
Accept-Encoding: gzip, deflate\r\n\
Connection: keep-alive\r\n\r\n"

#define POST_EXAMPLE                                                           \
  "POST /submit-form HTTP/1.1\r\n\
Host: www.example.com\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Content-Length: a\r\n\
\r\n\
username=johndoe&password=12345"

#define INVALID_EXAMPLE                                                        \
  "Host: www.example.com\r\n\
POST /submit-form HTTP/1.1\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Content-Length: 27\r\n\
\r\n\
username=johndoe&password=12345"

#define MESSAGE_END "\n*-------------------------*\n"

void test_parser();

int main() { test_parser(); }

void test_parser() {
  Request parser_src(GET_EXAMPLE);
  parser_src.parseRequest();

  Request parser = parser_src;
  /* parser.parseRequest(); */

  std::cout << "raw request:\n"
            << parser.get_rawRequest() << MESSAGE_END << std::endl;
  std::cout << "request method: " << parser.get_requestMethod() << std::endl;
  std::cout << "uri: " << parser.get_uri() << std::endl;
  std::cout << "html version: " << parser.get_htmlVersion() << std::endl;

  std::cout << "*** Headers ***" << std::endl;
  std::unordered_map<std::string, std::string> headers = parser.get_headers();

  std::unordered_map<std::string, std::string>::iterator it = headers.begin();
  while (it != headers.end()) {
    std::cout << (*it).first << ": " << (*it).second << std::endl;
    ++it;
  }
  std::cout << std::endl;

  std::cout << "*** Body ***" << std::endl;
  std::cout << "body: " << parser.get_body() << std::endl << std::endl;

  std::cout << "Validity check: " << std::endl;
  if (parser.checkRequestValidity())
    std::cout << "Valid!" << std::endl;
  else
    std::cout << "Invalid!" << std::endl;
}
