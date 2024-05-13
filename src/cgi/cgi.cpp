#include "cgi.hpp"
#include <iostream>

#define cgi_dir "/var/www/cgi-bin"

cgi::cgi() : _contentType(""), _title("") {
  std::cout << "CGI default constructor called." << std::endl;
}

cgi::cgi(const std::string &contentType, const std::string &title)
    : _contentType(contentType), _title(title) {
  std::cout << "CGI constructor called." << std::endl;
}

cgi::cgi(const std::string &title) : _title(title) {
  std::cout << "CGI constructor called." << std::endl;
}

cgi::~cgi() { std::cout << "CGI destructor called." << std::endl; }

std::string cgi::get_header(const std::string &content_type) {
  std::string header = "Content-type: " + content_type + "\r\n\r\n";
  return header;
}

std::string cgi::get_start_html(const std::string &title) {
  std::string html_body =
      "<html><head><title>" + title + "</title></head><body>";
  return html_body;
}

std::string cgi::get_end_html() { return "</body></html>"; }

void cgi::set_header(const std::string &content_type) {
  _contentType = content_type;
}

void cgi::set_title(const std::string &title) { _title = title; }
