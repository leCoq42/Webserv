#include "fileAccess.hpp"
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


//check for seg faults on random input
//return / error bodies. 

//ROOT rewrite, can be default or location specific, FIXED
//HTTP METHOD permissed FIXED
//redirect might have to do http to https, which is diffrent from the current implemented
// multiple server check
// move security struct up stream

FileAccess::FileAccess(std::list<ServerStruct> *config): config(config)
{
	std::cout << MSG_BORDER << "[FILEACCESS SETUP]" << MSG_BORDER << std::endl;
	_return = "";
}

FileAccess::~FileAccess() {}

//with asterix wildcards
bool	find_server_name_in_uri(std::string uri, std::string server_name)
{
	if (server_name.at(0) == '*' && uri.find(server_name.substr(1), (uri.length() - server_name.length())) == uri.length() - server_name.length() + 1)
		return (true);
	else if (server_name.at(server_name.length() - 1) == '*' && !uri.find(server_name.substr(0, server_name.length() - 1)))
		return (true);
	else if (!uri.find(server_name))
		return (true);
	return (false);
}

void	FileAccess::swap_to_right_server_config(std::string uri, int port)
{
	std::string		port_str;
	ServerStruct	*prev_match;

	prev_match = NULL;
	port_str = std::to_string(port);
	for (ServerStruct &server_config : *config)
	{
		for (std::string &port_config : server_config._port.content_list)
		{
			if (port_config == port_str)
			{
				if (!prev_match)
					prev_match = &server_config;
				else if (server_config._names.content_list.front().length() < prev_match->_names.content_list.front().length()
					&& find_server_name_in_uri(uri, server_config._names.content_list.front()))
					prev_match = &server_config;
			}
		}
	}
	server = prev_match;
	std::cout << "SELECTED: " << server->_id << std::endl; 
	_root = server->_root.content_list.back();
	_currentRoot = _root;
	_allowedMethods = &server->_allowMethods.content_list;
	_currentAllowedMethods = _allowedMethods;
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

bool			FileAccess::is_deleteable(std::filesystem::path to_delete)
{
	std::filesystem::path root;
	std::filesystem::path current_root;

	root = _root;
	current_root = _currentRoot;
	root = std::filesystem::absolute(root);
	current_root = std::filesystem::absolute(current_root);
	to_delete = std::filesystem::absolute(to_delete);
	if (to_delete.string().find(root.string()) == 0 && to_delete.string().find(root.string()) == 0)
	{
		std::cout << "delete" << std::endl;
		return true;
	}
	std::cout << "nono" << std::endl;
	return false;
}

ConfigContent	*FileAccess::find_location_config(std::string uri, ConfigContent *location_config)
{
	ConfigContent	*current;
	ConfigContent	*previous_match;
	int				match_type_requested;
	std::string		*loc_conf;
	std::string		new_uri;

	current = location_config;
	previous_match = NULL;
	new_uri = "/";
	new_uri.append(uri);
	uri = new_uri;
	while (current)
	{
		match_type_requested = get_match_type(current->content_list.front());
		loc_conf = &current->content_list.back();
		if (match_type_requested != MATCH_TYPE_POSTFIX 
			&& !uri.find(*loc_conf)
			&& uri.length() >= loc_conf->length())
		{
			if (match_type_requested == MATCH_TYPE_EXACT && !uri.compare(*loc_conf))
			{
				previous_match = current;
				break;
			}
			else
			{
				if (!previous_match || previous_match->content_list.back().length() < loc_conf->length())
					previous_match = current;
			}
		}
		else if (match_type_requested == MATCH_TYPE_POSTFIX)
		{
			std::cout << "post+fix" << std::endl;
			std::cout << *loc_conf << "==" << uri << "_" << uri.find(*loc_conf, (uri.length() - loc_conf->length())) << "=" << uri.length() - loc_conf->length() << std::endl;
			if (uri.find(*loc_conf, (uri.length() - loc_conf->length())) == uri.length() - loc_conf->length())
			{
				previous_match = current;
				break;
			}
		}
		current = current->next;
	}
	if ((LocationStruct *)previous_match->childs && !((LocationStruct *)previous_match->childs)->allow_methods.content_list.empty())
		_currentAllowedMethods = &((LocationStruct *)previous_match->childs)->allow_methods.content_list;
	return (previous_match);
}

std::string	FileAccess::swap_out_root(std::string uri, ConfigContent *location_config, std::string root)
{
	std::filesystem::path	root_swapped_path;

	if (location_config->childs && !((LocationStruct *)location_config->childs)->root.content_list.empty())
		root = ((LocationStruct *)location_config->childs)->root.content_list.back();
	root_swapped_path = root;
	if (!uri.empty())
		root_swapped_path.append(uri);
	return (root_swapped_path);
}

std::filesystem::path FileAccess::uri_is_directory(std::string uri, ConfigContent *location_config, int &return_code)
{
	std::filesystem::path	path;

	path = uri;
	if (!path.extension().empty())
		return (path);
	if (!((LocationStruct *)location_config->childs)->index.content_list.empty())
	{
		path.append(((LocationStruct *)location_config->childs)->index.content_list.back());
		return (path);
	}
	if (!((LocationStruct *)location_config->childs)->autoindex.content_list.back().compare("on"))
		return (path);
	return_code = 404;
	return ("");	
}

std::string	FileAccess::redirect(int &return_code)
{
	if (!server->_return.content_list.empty())
	{
		return_code = 301;
		_return = server->_return.content_list.back();
		return ("");
	}
	return ("");
}

//NEW FUNCTION:
std::filesystem::path	FileAccess::isFilePermissioned(std::string uri, int &return_code, int port)
{
	std::string		new_uri;
	ConfigContent	*location_config;
	std::filesystem::path	path;

	//find server_name in config and swap to that config.
	swap_to_right_server_config(uri, port);

	//check if redirect, if so return redirect.
	new_uri = redirect(return_code);
	if (return_code == 301)
		return ("/"); //reset uri to new_uri
	// //find location config. check allowedMethod and if not allowed return
	location_config = &server->_location;
	location_config = find_location_config(uri, location_config);
	if (location_config && !((LocationStruct *)location_config->childs)->allow_methods.content_list.empty())
		_currentAllowedMethods = &((LocationStruct *)location_config->childs)->allow_methods.content_list;
	else
		_currentAllowedMethods = _allowedMethods;
	// //swap out root.
	new_uri = swap_out_root(uri, location_config, _root);
	// check if request is a directory and perfect match. if so swap to index_file, if not perfect or no index_file match auto index, 404 of auto index is turned off.
	path = uri_is_directory(new_uri, location_config, return_code);
	return (path);
}

//Get configured error_page path by inputting error number, this is fine
std::filesystem::path	FileAccess::getErrorPage(int return_code)
{
	ConfigContent	*current;
	std::string		error_code;

	error_code = std::to_string(return_code);
	current = &server->_errorPage;
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
	std::cout << "requested method " << method << std::endl;
	for (std::string content : *_currentAllowedMethods)
	{
		if (!content.compare(method))
			return true;
	}
	std::cout << "declined" << std::endl;
	return false;
}

std::string	FileAccess::get_return(void)
{
	return (_return);
}
