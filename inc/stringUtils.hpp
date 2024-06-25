#pragma once

#include "webserv.hpp"

std::string trim(const std::string &str, const std::string &tokens);

std::vector<std::string> split(const std::string &str,
                               const std::string &delim);
