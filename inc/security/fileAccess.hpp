#pragma once
#include "Parser.hpp"
#include "ServerStruct.hpp"
#include "LocationStruct.hpp"
// #include "response.hpp"
#include <filesystem>
#include <vector>

class	Response;

class	FileAccess
{
	private:
	std::filesystem::path				_root;
	std::filesystem::path				_currentRoot;
	std::list<std::string>				_allowedMethods;
	std::list<std::string>				_currentAllowedMethods;
	ServerStruct						&_config;
	std::vector<std::filesystem::path>	_uploadedFiles;
	std::vector<std::filesystem::path>	_configPaths;
	std::vector<LocationStruct>			_configLocations;

	public:
	FileAccess(ServerStruct &config);
	~FileAccess();
	std::filesystem::path	root_or_path(std::filesystem::path path, std::filesystem::path current_root, std::filesystem::path root, LocationStruct	*current);
	std::filesystem::path	isFilePermissioned(std::string uri, int &return_code);
	std::filesystem::path	find_location(std::filesystem::path path, std::string uri, int &return_code);
	std::filesystem::path	getErrorPage(int return_code);
	bool					allowedMethod(std::string method);
	void					addFile(std::filesystem::path file);
	void					show_all_allowed(void);
};
