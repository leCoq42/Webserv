#include "cgi.hpp"

int main() {
  cgi CGI("text/html", "Clock");
  CGI.executeCGI("/home/mhaan/core/webserv/src/cgi/time.cgi", "");

  return 0;
}
