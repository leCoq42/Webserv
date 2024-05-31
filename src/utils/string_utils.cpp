#include "Webserv.hpp"

std::string trim(const std::string &str, const std::string &tokens) {
  if (str.empty())
    return str;
  size_t start = str.find_first_not_of(tokens);
  size_t end = str.find_last_not_of(tokens);
  if (start == std::string::npos || end == std::string::npos)
    return "";
  // std::cout << "Trimmed:" << str.substr(start, end - start + 1) << std::endl;
  return str.substr(start, end - start + 1);
}
