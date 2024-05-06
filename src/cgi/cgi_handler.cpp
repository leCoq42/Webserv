#include "cgi_handler.hpp"
#include <iostream>

#define cgi_dir "/var/www/cgi-bin"

cgi_handler::cgi_handler() {
  std::cout << "CGI constructor called." << std::endl;
}

cgi_handler::~cgi_handler() {
  std::cout << "CGI destructor called." << std::endl;
}
