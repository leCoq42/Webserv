#include <iostream>
#include <vector>
#include "log.hpp"

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

std::vector<std::string> split(const std::string &str,
                               const std::string &delim) {
  std::vector<std::string> tokens;
  size_t pos = 0;
  size_t next = 0;

  while (pos != std::string::npos) {
    next = str.find(delim, pos);
    if (next == std::string::npos) {
      tokens.push_back(str.substr(pos));
      break;
    }
    tokens.push_back(str.substr(pos, next - pos));
    pos = next + delim.length();
  }
  return tokens;
}
