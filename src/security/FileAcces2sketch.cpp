#include "FileAcces.hpp"
#include "defines.hpp"

//ROOT removes location and replaces it with the given path
//INDEX swaps location with the given index file
//AUTO_INDEX runs on request of a directory
//REDIRECT, return 301 and defined link when server is requested
//SERVER_NAME compares to first part of requested path and switches config accordingly
//CGI define cgi path accoriding to extension
//UPLOAD_LOCATION configure where the uploads should be saved
//DEFAULT_ERROR_PAGES link error_code to error_page *implemented correctly
//LIMIT CLIENT BODY SIZE has to be loaded into the buffering size
//DEFINE ACCEPTED METHODS implemented correctly


//ROOT rewrite, can be default or location specific, FIXED
//HTTP METHOD permissed FIXED
//redirect might have to do http to https, which is diffrent from the current implemented
// multiple server check
// move security struct up stream

FileAccess::FileAccess(ServerStruct &config): config(config)
{
	std::cout << MSG_BORDER << "[FILEACCESS SETUP]" << MSG_BORDER << std::endl;
	root = config._root.content_list.back();//configPaths.push_back(config.root.content_list.back());
	current_root = root;
	allowed_methods = config._allowMethods.content_list;
	// show_all_allowed();
}

FileAccess::~FileAccess() {}

// //Swaps out root if different root is given. And checks if path is within root.
// std::filesystem::path	FileAccess::root_or_path(std::filesystem::path path, std::filesystem::path current_root, std::filesystem::path root, LocationStruct	*current)
// {
// 	std::filesystem::path	root_swapped_path;
// 	std::string				non_root;

// 	if (current && !current->allow_methods.content_list.empty())
// 		current_allowed_methods = current->allow_methods.content_list;
// 	else
// 		current_allowed_methods = allowed_methods;
// 	if ((path.string().find_first_of(root) + root.string().length() + 1) < path.string().length())
// 	{
// 		non_root = path.string().substr(path.string().find_first_of(root) + root.string().length() + 1);
// 		root_swapped_path = current_root;
// 		root_swapped_path.append(non_root);
// 	}
// 	else
// 		root_swapped_path = path;
// 	if (path.string().find_first_of(current_root) != std::string::npos)
// 		return (root_swapped_path);
// 	return (current_root);
// }

// //Checks if location is present in config file. and will rewrite the path according to the config file setup. 
// std::filesystem::path	FileAccess::find_location(std::filesystem::path path, std::string uri, int &return_code)
// {
// 	ConfigContent	*current;

// 	current = &config._location;
// 	current_root = root;
// 	uri.insert(0, "/");
// 	// std::cout << "find location:" << uri << "}{" << uri.substr(0, uri.find_last_of("/")) << std::endl;
// 	if (path.string().find_last_of(root) != std::string::npos)
// 	{
// 		while (current)
// 		{
// 			for (std::string content : current->content_list)
// 			{
// 				if (!((LocationStruct *)current->childs)->root.content_list.empty())
// 					current_root.append(((std::string)((LocationStruct *)current->childs)->root.content_list.back()));

// 				// std::cout << "Try Path:" << current_root << std::endl;
// 				// std::cout << "Requested:" << uri << "=" << content << std::endl;
// 				if (path.has_extension())
// 				{
// 					if (!uri.substr(0, uri.find_last_of("/")).compare(content) || !uri.substr(0, uri.find_last_of("/")).compare(""))
// 						return (root_or_path(path, current_root, root, (LocationStruct *)current->childs));
// 				}
// 				if (!uri.compare(content))
// 				{
// 					if (!((LocationStruct *)current->childs)->_return.content_list.empty())
// 					{
// 						// std::cout << "Redirecting\n";
// 						path = current_root;
// 						path.append(((LocationStruct *)current->childs)->_return.content_list.back());
// 						return (find_location(path, ((LocationStruct *)current->childs)->_return.content_list.back(), return_code));
// 					}
// 					if (!((LocationStruct *)current->childs)->autoindex.content_list.back().compare("on"))
// 						return (root_or_path(path, current_root, root, (LocationStruct *)current->childs));
// 					else
// 					{
// 						path.append(((LocationStruct *)current->childs)->index.content_list.back());
// 						return (root_or_path(path, current_root, root, (LocationStruct *)current->childs));
// 					}
// 				}
// 			}
// 			current = current->next;
// 		}
// 	}
// 	return_code = 404;
// 	std::cout << "File Access: nothing found!\n";
// 	return (root_or_path(root, root, root, NULL));
// }

//Use this function to check if requested uri is okay to try and reach. it will return an edited path according to config files
std::filesystem::path	FileAccess::isFilePermissioned(std::string uri, int &return_code)
{
	std::filesystem::path	path;

	path = root;
	path.append(uri);
	//look through locations in server
	path = find_location(path, uri, return_code);
	// std::cout << return_code << ":error\n";
	// std::cout << config._root.content_list.back();
	return (path);
}

//with asterix wildcards
bool	find_server_name_in_uri(std::string uri, std::string server_name)
{
	if (server_name.at(0) == '*' && !uri.find_first_of(server_name, (uri.str_len() - server_name.str_len())))
		return (true);
	else if (server_name.at(server_name.strlen() - 1) == '*' && !uri.find_first_of(server_name))
		return (true);
	else if (!uri.compare(server_name))
		return (true);
	return (false);
}

//source: https://www.digitalocean.com/community/tutorials/nginx-location-directive
#define MATCH_TYPE_UNSPECIFIED 0
#define MATCH_TYPE_EXACT 1
#define MATCH_TYPE_POSTFIX 2

int	get_match_type(std::string first_content_part) //could be switch
{
	if (!first_content_part.compare("="))
		return (MATCH_TYPE_EXACT);
	else if (!first_content_part.compare("*=")) //self specified, regex like to get extension specific behaviour
		return (MATCH_TYPE_POSTFIX);
	else
		return (MATCH_TYPE_UNSPECIFIED);
}

ConfigContent	*find_location_config(std::string uri)
{
	ConfigContent	*current;
	ConfifContent	*previous_match;
	int				match_type_requested;
	std::string		*loc_conf;

	current = &config._location;
	previous_match = NULL;
	while (current)
	{
		match_type_requested = get_match_type(current->content_list.first());
		loc_conf = &current->content_list.last();
		if (match_type_requested != MATCH_TYPE_POSTFIX && !uri.find_first_of(*loc_conf))
		{
			if (match_type_requested == MATCH_TYPE_EXACT && !uri.compare(*loc_conf))
				return (current);
			else
			{
				if (!previous_match || previous_match->content_list.last().str_len() < loc_conf->str_len())
					previous_match = current;
			}
		}
		else if (match_type_requested == MATCH_TYPE_POSTFIX && uri.find_first_of(*loc_conf, (uri.str_len() - loc_conf->str_len())))
			return (current);
	}
	return (previous_match);
}

std::string	swap_out_root(std::string uri, ConfigContent *location_config, std::string root)
{
	std::string	root_swapped_uri;

	if (!location_config->_root.content_list.empty())
		root = location_config->_root.content_list.back();
	root_swapped_uri = root;
	root_swapped_uri.append(uri.substr(uri.find_last_of(location_config->content_list.last())));
	return (root_swapped_uri);
}

std::filesystem::path uri_is_directory(std::string uri, ConfigContent *location_config, int &return_code)
{
	std::filesystem::path	path;

	path = uri;
	if (!path.extension().empty())
		return (path);
	if ((!(LocationStruct *)current->childs)->index.content_list.empty())
	{
		path.append(((LocationStruct *)current->childs)->index.content_list.back());
		return (path);
	}
	if (((LocationStruct *)current->childs)->autoindex.content_list.back().compare("on"))
		return (path);
	return_code = 404;
	return (path);	
}

std::string	redirect(ConfigContent *location_config, int &return_code)
{
	if (!((LocationStruct *)location_config->childs)->_return.content_list.empty())
	{
		return_code = 301;
		return (((LocationStruct *)location_config->childs)->_return.content_list.back());
	}
	return ("");
}

//NEW FUNCTION:
std::filesystem::path	FileAccess::isFilePermissioned(std::string uri, int &return_code)
{
	std::string		new_uri;
	ConfigContent	*server_struct;
	ConfigContent	*location_config;

	//find server_name in config and swap to that config.
	// location_config = find_location_config(uri);
	//check if redirect, if so return redirect.
	new_uri = redirect(location_config, return_code);
	if (return_code = 301)
		return (NULL); //reset uri to new_uri
	//find location config. check allowedMethod and if not allowed return
	location_config = find_location_config(uri);
	//swap out root.
	new_uri = swap_out_root(uri, location_config, root);
	//check if cgi, if so check allowedMethod and return.
	//check if request is a directory and perfect match. if so swap to index_file, if not perfect or no index_file match auto index.
	return (uri_is_directory(new_uri, location_config, return_code));
}

//Get configured error_page path by inputting error number, this is fine
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

//check if method is allowed for this specific location, this is fine
bool	FileAccess::allowedMethod(std::string method)
{
	for (std::string content : current_allowed_methods)
	{
		if (!content.compare(method))
			return true;
	}
	return false;
}

// void	FileAccess::show_all_allowed(void)
// {
// 	int	i;

// 	// check if in root
// 	std::cout << "SHOW ALL UPLOADS:" << std::endl;
// 	i = uploadedFiles.size();
// 	while (i--)
// 		std::cout << uploadedFiles[i] << std::endl;
// 	std::cout << "Root:" << root << std::endl;
// }
