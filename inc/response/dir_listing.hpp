#include <dirent.h>
#include <iostream>
#include <filesystem>

std::string list_dir(std::filesystem::path &path, const std::string &uri, const std::string &referer);