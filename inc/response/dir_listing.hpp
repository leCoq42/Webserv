#include <dirent.h>
#include <iostream>
#include <filesystem>

std::string list_dir(std::filesystem::path &path, const std::string &uri, const std::string &referer, int &status_code);
std::string	redirect(std::string redirect_string);
std::string	standard_error(int error_code, std::string error_description);