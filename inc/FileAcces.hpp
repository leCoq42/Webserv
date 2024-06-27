#pragma once
#include "Parser.hpp"
#include "ServerStruct.hpp"
#include "LocationStruct.hpp"
// #include "response.hpp"
#include <filesystem>
#include <vector>

class	Response;

class	FileAcces
{
	private:
	std::filesystem::path				root;
	std::filesystem::path				current_root;
	std::list<std::string>				allowed_methods;
	std::list<std::string>				current_allowed_methods;
	ServerStruct						&config;
	std::vector<std::filesystem::path>	uploadedFiles;
	std::vector<std::filesystem::path>	configPaths;
	std::vector<LocationStruct>			configLocations;
	public:
	FileAcces(ServerStruct &config);
	~FileAcces();
	std::filesystem::path	root_or_path(std::filesystem::path path, std::filesystem::path current_root, std::filesystem::path root, LocationStruct	*current);
	std::filesystem::path	isFilePermissioned(std::string uri, int &return_code);
	std::filesystem::path	find_location(std::filesystem::path path, std::string uri, int &return_code);
	std::filesystem::path	getErrorPage(int return_code);
	bool					allowedMethod(std::string method);
	void	addFile(std::filesystem::path file);
	void	show_all_allowed(void);
};
