#include "cgi.hpp"
#include <iostream>

#define cgi_dir "/var/www/cgi-bin"

cgi::cgi() { std::cout << "CGI constructor called." << std::endl; }

cgi::~cgi() { std::cout << "CGI destructor called." << std::endl; }
