#include "cgi.hpp"

int main() {
  cgi CGI("text/html", "Clock");

  std::cout << CGI.get_header(CGI.get_contentType());
  std::cout << CGI.get_start_html(CGI.get_title());
  // std::cout << "test" << std::endl;

  CGI.executeCGI("/home/mhaan/core/webserv/src/cgi/time", "");
  std::cout << CGI.get_end_html();

  return 0;
}
