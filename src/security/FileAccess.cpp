#include "fileAccess.hpp"
#include "defines.hpp"

//ROOT rewrite, can be default or location specific, FIXED
//HTTP METHOD permissed FIXED
//redirect might have to do http to https, which is diffrent from the current implemented
// multiple server check
// move security struct up stream

FileAccess::FileAccess(ServerStruct &config): _config(config)
{
	std::cout << MSG_BORDER << "[FILEACCESS SETUP]" << MSG_BORDER << std::endl;
	_root = config._root.content_list.back();//configPaths.push_back(config.root.content_list.back());
	_currentRoot = _root;
	_allowedMethods = config._allowMethods.content_list;
	// show_all_allowed();
}

FileAccess::~FileAccess() {}

//Swaps out root if different root is given. And checks if path is within root.
std::filesystem::path	FileAccess::root_or_path(std::filesystem::path path, std::filesystem::path current_root, std::filesystem::path root, LocationStruct *current)
{
	std::filesystem::path	root_swapped_path;
	std::string				non_root;

	if (current && !current->allow_methods.content_list.empty())
		_currentAllowedMethods = current->allow_methods.content_list;
	else
		_currentAllowedMethods = _allowedMethods;

	if ((path.string().find_first_of(root) + root.string().length() + 1) < path.string().length())
	{
		non_root = path.string().substr(path.string().find_first_of(root) + root.string().length() + 1);
		root_swapped_path = current_root;
		root_swapped_path.append(non_root);
	}
	else
		root_swapped_path = path;

	if (path.string().find_first_of(current_root) != std::string::npos)
		return (root_swapped_path);
	return (current_root);
}

//Checks if location is present in config file. and will rewrite the path according to the config file setup. 
std::filesystem::path	FileAccess::find_location(std::filesystem::path path, std::filesystem::path uri, int &return_code) {
	ConfigContent	*current;

	current = &_config._location;
	_currentRoot = _root;
	// uri.string().insert(0, "/");
	// std::cout << "find location:" << uri << "}{" << uri.substr(0, uri.find_last_of("/")) << std::endl;
	if (path.parent_path().string().find_last_of(_root) != std::string::npos)
	{
		while (current)
		{
			for (std::string content : current->content_list)
			{
				if (!((LocationStruct *)current->childs)->root.content_list.empty()) {
					std::cout << "test1" << std::endl;
					_currentRoot.append(((std::string)((LocationStruct *)current->childs)->root.content_list.back()));
				}

				std::cout << "Try Path:" << _currentRoot << std::endl;
				std::cout << "Requested: " << uri << " = " << content << std::endl;

				if (path.has_extension())
				{
					std::cout << "test2" << std::endl;
					if (uri.has_parent_path() || uri.parent_path() != content) {
						std::cout << "test3" << std::endl;
						std::cout << "parent path = " << uri.parent_path() << std::endl;
						std::cout << "content = " << content << std::endl;
						return (root_or_path(path, _currentRoot, _root, (LocationStruct *)current->childs));
					}
					// if (!uri.substr(0, uri.find_last_of("/")).compare(content) || !uri.substr(0, uri.find_last_of("/")).compare("")) {
					// 	return (root_or_path(path, _currentRoot, _root, (LocationStruct *)current->childs));
					// }
				}
				if (!uri.compare(content)) {

					std::cout << "test4" << std::endl;
					if (!((LocationStruct *)current->childs)->_return.content_list.empty()) {
						std::cout << "test5" << std::endl;
						std::cout << "Redirecting" << std::endl;
						path = _currentRoot;
						path.append(((LocationStruct *)current->childs)->_return.content_list.back());
						return (find_location(path, ((LocationStruct *)current->childs)->_return.content_list.back(), return_code));
					}
					if (!((LocationStruct *)current->childs)->autoindex.content_list.back().compare("on")) {
						std::cout << "test6" << std::endl;
						return (root_or_path(path, _currentRoot, _root, (LocationStruct *)current->childs));
					}
					else {
						std::cout << "test7" << std::endl;
						path.append(((LocationStruct *)current->childs)->index.content_list.back());
						return (root_or_path(path, _currentRoot, _root, (LocationStruct *)current->childs));
					}
				}
			}
			current = current->next;
		}
	}
	return_code = 404;
	std::cout << "File Access: nothing found!" << std::endl;
	return (root_or_path(_root, _root, _root, NULL));
}

//Use this function to check if requested uri is okay to try and reach. it will return an edited path according to config files
std::filesystem::path	FileAccess::isFilePermissioned(std::filesystem::path uri, int &return_code)
{
	std::filesystem::path	path;

	path = _root;
	path /= uri;
	//look through locations in server
	path = find_location(path, uri, return_code);
	// std::cout << return_code << ":error\n";
	// std::cout << config._root.content_list.back();
	return (path);
}

//Get configured error_page path by inputting error number
std::filesystem::path	FileAccess::getErrorPage(int return_code)
{
	ConfigContent	*current;
	std::string		error_code;

	error_code = std::to_string(return_code);
	current = &_config._errorPage;
	while (current)
	{
		if (!current->content_list.front().compare(error_code))
			return (current->content_list.back());
		current = current->next;
	}
	return ("/");
}

//check if method is allowed for this specific location
bool	FileAccess::allowedMethod(std::string method)
{
	for (std::string content : _currentAllowedMethods)
	{
		if (!content.compare(method))
			return true;
	}
	return false;
}

void	FileAccess::show_all_allowed(void)
{
	int	i;

	// check if in root
	std::cout << "SHOW ALL UPLOADS:" << std::endl;
	i = _uploadedFiles.size();
	while (i--)
		std::cout << _uploadedFiles[i] << std::endl;
	std::cout << "Root:" << _root << std::endl;
}
