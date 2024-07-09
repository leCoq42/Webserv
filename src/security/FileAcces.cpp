#include "FileAcces.hpp"
#include "defines.hpp"

//ROOT rewrite, can be default or location specific, FIXED
//HTTP METHOD permissed FIXED
//redirect might have to do http to https, which is diffrent from the current implemented
// multiple server check
// move security struct up stream

FileAccess::FileAccess(ServerStruct &config): config(config)
{
	std::cout << MSG_BORDER << "[FILEACCES SETUP]" << MSG_BORDER << std::endl;
	root = config._root.content_list.back();//configPaths.push_back(config.root.content_list.back());
	current_root = root;
	allowed_methods = config._allowMethods.content_list;
	show_all_allowed();
}

FileAccess::~FileAccess() {}

//Swaps out root if different root is given. And checks if path is within root.
std::filesystem::path	FileAccess::root_or_path(std::filesystem::path path, std::filesystem::path current_root, std::filesystem::path root, LocationStruct	*current)
{
	std::filesystem::path	root_swapped_path;
	std::string				non_root;

	if (current && !current->allow_methods.content_list.empty())
		current_allowed_methods = current->allow_methods.content_list;
	else
		current_allowed_methods = allowed_methods;
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
std::filesystem::path	FileAccess::find_location(std::filesystem::path path, std::string uri, int &return_code)
{
	ConfigContent	*current;

	current = &config._location;
	current_root = root;
	uri.insert(0, "/");
	std::cout << "find location:" << uri << "}{" << uri.substr(0, uri.find_last_of("/")) << std::endl;
	if (path.string().find_last_of(root) != std::string::npos)
	{
		while (current)
		{
			for (std::string content : current->content_list)
			{
				if (!((LocationStruct *)current->childs)->root.content_list.empty())
					current_root.append(((std::string)((LocationStruct *)current->childs)->root.content_list.back()));

				std::cout << "Try Path:" << current_root << std::endl;
				std::cout << "Requested:" << uri << "=" << content << std::endl;
				if (path.has_extension())
				{
					if (!uri.substr(0, uri.find_last_of("/")).compare(content) || !uri.substr(0, uri.find_last_of("/")).compare(""))
						return (root_or_path(path, current_root, root, (LocationStruct *)current->childs));
				}
				if (!uri.compare(content))
				{
					if (!((LocationStruct *)current->childs)->_return.content_list.empty())
					{
						std::cout << "Redirecting\n";
						path = current_root;
						path.append(((LocationStruct *)current->childs)->_return.content_list.back());
						return (find_location(path, ((LocationStruct *)current->childs)->_return.content_list.back(), return_code));
					}
					if (!((LocationStruct *)current->childs)->autoindex.content_list.back().compare("on"))
						return (root_or_path(path, current_root, root, (LocationStruct *)current->childs));
					else
					{
						path.append(((LocationStruct *)current->childs)->index.content_list.back());
						return (root_or_path(path, current_root, root, (LocationStruct *)current->childs));
					}
				}
			}
			current = current->next;
		}
	}
	return_code = 404;
	std::cout << "File Access: nothing found!\n";
	return (root_or_path(root, root, root, NULL));
}

//Use this function to check if requested uri is okay to try and reach. it will return an edited path according to config files
std::filesystem::path	FileAccess::isFilePermissioned(std::string uri, int &return_code)
{
	std::filesystem::path	path;

	path = root;
	path.append(uri);
	//look through locations in server
	path = find_location(path, uri, return_code);
	std::cout << return_code << ":error\n";
	std::cout << config._root.content_list.back();
	return (path);
}

//Get configured error_page path by inputting error number
std::filesystem::path	FileAccess::getErrorPage(int return_code)
{
	ConfigContent	*current;
	std::string		error_code;

	error_code = std::to_string(return_code);
	current = &config._errorPage;
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
	for (std::string content : current_allowed_methods)
	{
		if (!content.compare(method))
			return true;
	}
	return false;
}

void	FileAccess::show_all_allowed(void)
{
	int	i;

	//check if in root
	std::cout << "SHOW ALL UPLOADS:" << std::endl;
	i = uploadedFiles.size();
	while (i--)
		std::cout << uploadedFiles[i] << std::endl;
	std::cout << "Root:" << root << std::endl;
}
