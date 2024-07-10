#pragma once

#include <iostream>
#include <vector>

std::string trim(const std::string &str, const std::string &tokens);

std::vector<std::string> split(const std::string &str,
                               const std::string &delim);
