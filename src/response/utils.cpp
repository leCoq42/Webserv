#include <iostream>

// Function to trim whitespace from both ends of a string
std::string trim(const std::string &str) {
  std::string whitespace(" \t\f\v\n\r");

  size_t first = str.find_first_not_of(whitespace);
  if (std::string::npos == first)
    return str;

  size_t last = str.find_last_not_of(whitespace);
  return str.substr(first, (last - first + 1));
}
