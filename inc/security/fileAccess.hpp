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
	std::string							_return;
	std::list<std::string>				*_allowedMethods;
	std::list<std::string>				*_currentAllowedMethods;
	std::list<ServerStruct>				*config;
	ServerStruct						*server;
	std::vector<std::filesystem::path>	_uploadedFiles;
	std::vector<std::filesystem::path>	_configPaths;
	std::vector<LocationStruct>			_configLocations;

	public:
	FileAccess(std::list<ServerStruct> *config);
	~FileAccess();
	std::filesystem::path	root_or_path(std::filesystem::path path, std::filesystem::path current_root, std::filesystem::path root, LocationStruct	*current);
	// std::filesystem::path	isFilePermissioned(std::string uri, int &return_code);
	std::filesystem::path	isFilePermissioned(std::string uri, int &return_code, int port, std::string method);
	// std::filesystem::path	find_location(std::filesystem::path path, std::string uri, int &return_code);
	std::filesystem::path	find_location(std::filesystem::path path, std::string uri, int &return_code);
	std::filesystem::path	getErrorPage(int return_code);
	bool					allowedMethod(std::string method);
	void					addFile(std::filesystem::path file);
	void					swap_to_right_server_config(std::string uri, int port);
	void					show_all_allowed(void);
	std::string				redirect(int &return_code);
	ConfigContent			*find_location_config(std::string uri, ConfigContent *location_config, std::string method);
	std::string				swap_out_root(std::string uri, ConfigContent *location_config, std::string root);
	std::filesystem::path 	uri_is_directory(std::string uri, ConfigContent *location_config, int &return_code);
	bool					is_deleteable(std::filesystem::path to_delete);
	std::string				get_return(void);
};
