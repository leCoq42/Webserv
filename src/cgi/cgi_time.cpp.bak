#include "cgi.hpp"
#include <ctime>
#include <iostream>

int main() {
  cgi CGI("text/html", "Time");

  std::cout << CGI.get_header(CGI.get_contentType());
  std::cout << CGI.get_start_html(CGI.get_title());

  time_t currentTime; // time_t defined in <ctime>
  time(&currentTime);
  std::cout << asctime(localtime(&currentTime));

  std::cout << CGI.get_end_html();
  return 0;
}
