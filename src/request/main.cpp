#include "Request.hpp"
#include <iostream>
#include <ostream>

#define GET_EXAMPLE                                                            \
  "GET /index.html HTTP/1.1\r\nHost: "                                         \
  "www.example.com\r\nUser-Agent:Mozilla/5.0 (Windows NT 10.0; Win64; "        \
  "x64)\r\nAccept: "                                                           \
  "text/html,application/xhtml+xml,application/xml;q=0.9,*/"                   \
  "*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, "      \
  "deflate\r\nConnection: keep-alive\r\n\r\n"

#define POST_EXAMPLE                                                           \
  "POST /submit-form HTTP/1.1\r\n\
Host:www.example.com\r\n\
Content-Type:application/x-www-form-urlencoded\r\n\
Content-Length:31\r\n\r\n\
username=johndoe&password=12345"

#define INVALID_EXAMPLE                                                        \
  "Host: www.example.com\r\n\
POST /submit-form HTTP/1.1\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Content-Length: 27\r\n\r\n\
username=johndoe&password=12345"

#define MESSAGE_END "\n*-------------------------*\n"

void test_parser(std::string raw_request);

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  test_parser(POST_EXAMPLE);
}

void test_parser(std::string raw_request) {
  Request parser(raw_request);

  // Request parser(parser_src);

  parser.printRequest();
}
