#include "CgiParsing.hpp"

std::vector<std::string>	meta_variables_names = {"AUTH_TYPE" , "CONTENT_LENGTH" ,
                           "CONTENT_TYPE" , "GATEWAY_INTERFACE" ,
                           "PATH_INFO" , "PATH_TRANSLATED" ,
                           "QUERY_STRING" , "REMOTE_ADDR" ,
                           "REMOTE_HOST" , "REMOTE_IDENT" ,
                           "REMOTE_USER" , "REQUEST_METHOD" ,
                           "SCRIPT_NAME" , "SERVER_NAME" ,
                           "SERVER_PORT" , "SERVER_PROTOCOL" ,
                           "SERVER_SOFTWARE" };
						   
bool	CgiParsing::dismantle_body(std::string body, std::string boundary)
{
	std::string	contentDisposition;
	std::string	contentType;

	if (boundary.empty())
		;
	// std::cout << "BOUNDARY:" << boundary << std::endl;
	// std::cout << "BODY:" << std::endl << body << std::endl;
	// std::cout << "End of BODY." << std::endl;
	// contentDisposition = body.substr(body.find("Content-Disposition: "), body.substr(body.find("Content-Disposition: ")).find("\n"));
	// std::cout << "CONTENT-DISPOSITION:" << contentDisposition << std::endl;
	// contentType = body.substr(body.find("Content-Type: "), body.substr(body.find("Content-Type: ")).find("\n"));
	// std::cout << "CONTENT-TYPE:" << contentType << std::endl;
	// body = body.find("\r\n\r\n");
	// std::cout << "body:" << body << std::endl;
	this->body = body;
	return (false);
}


//$_GET['name'] = hoi php file is argv name=hoi :: $_SERVER['name'] = doei php file is env var name=doei :: educated guess is that _POST['var'] = something goes into the stdin var=something
CgiParsing::CgiParsing(std::unordered_map<std::string, std::string> headers, char **environ, std::shared_ptr<Request> _request, const std::string &path, const std::string &interpreter) //ServerStruct &serverinfo
{
	int i;

	if (environ)
		;
	customizable_variables_names.push_back("X_");
	//config specified
	customizable_variables_names.push_back("");

	// std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GHFSGD<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
	i = -1;
	// std::cout << "ADD to ENVPP" << std::endl;
	// if (!interpreter.compare("/usr/bin/php-cgi"))
	// 	add_to_envpp("REDIRECT_STATUS", "", ""); //Security restriction thrown
	//MAGIC env variables for PHP REDIRECT_STATUS=true SCRIPT_FILENAME=/var/www/... REQUEST_METHOD=POST GATEWAY_INTERFACE=CGI/1.1
	if (!_request->get_requestMethod().compare("POST"))
	{
		add_to_envpp("REQUEST_METHOD", _request->get_requestMethod(), "");
		add_to_envpp("GATEWAY_INTERFACE", "CGI/1.1", "");
	}
	add_to_envpp("REDIRECT_STATUS", "true", "");
	add_to_envpp("SCRIPT_FILENAME", path, "");
	// add_to_envpp("GATEWAY_INTERFACE", "CGI/1.1", "");
	// while (*(environ + ++i))
	// 	add_to_envpp(((std::string)*(environ + i)).substr(0, ((std::string)*(environ + i)).find("=")), ((std::string)*(environ + i)).substr(((std::string)*(environ + i)).find("=") + 1), "");
	for (const auto& [key, value] : headers)
		add_to_envpp(key, value, "");
	// std::cout << "ADD to URI" << std::endl;
	if (interpreter.compare(""))
		add_to_uri(interpreter, "", "");
	add_to_uri(path, "", "");
	for (const auto& var : _request->get_requestArgs())
		add_to_uri(var, "", "");
	// std::cout << "ADD to BODY" <<std::endl;
	// std::cout << _request->get_body();
	dismantle_body(_request->get_body(), _request->get_boundary());
	// std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GHFSGD<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
}

CgiParsing::~CgiParsing(void)
{

}

bool	validate_key(std::string key, std::vector<std::string> customizable_variables_names)
{
	int	i;

	for (auto & c: key) c = toupper(c);
	i = meta_variables_names.size();
	while (i--)
		if (!meta_variables_names[i].compare(key))
			return true;
	i = customizable_variables_names.size();
	while (i--)
		if (key.find(customizable_variables_names[i]) != std::string::npos)
			return true;
	return false;
}

bool	CgiParsing::add_to_envpp(std::string name, std::string value, std::string additive)
{
	std::string	temp;
	// std::cout << name << "=" << value;
	if (validate_key(additive + name, customizable_variables_names))
	{
		// std::cout << name << "=" << value;
		temp = additive + name;
		for (auto & c: temp) c = toupper(c);
		if (value.compare(""))
			temp += "=" + value;
		// std::cout << temp << std::endl;
		std::replace( temp.begin(), temp.end(), '-', '_');
		// std::cout << temp << std::endl;
		meta_variables.push_back(temp);//uri[name] = value;
		// std::cout << " added" << std::endl;
		return true;
	}
	// std::cout << " failed" << std::endl;
	return false;
}

bool	CgiParsing::add_to_uri(std::string name, std::string value, std::string additive)
{
	std::string	temp;
	// std::cout << name << "=" << value;
	// if (!validate_key(additive + name, customizable_variables_names))
	// {
		// std::cout << name << "=" << value;
		temp = additive + name;
		if (value.compare(""))
			temp += "=" + value;
		uri.push_back(temp);//uri[name] = value;
		// std::cout << " added" << std::endl;
		return true;
	// }
	// std::cout << " failed" << std::endl;
	// return false;
}

std::vector<std::string>	&CgiParsing::get_argv()
{
	return (uri);
}

std::vector<std::string>	&CgiParsing::get_envp()
{
	return (meta_variables);
}

std::string					&CgiParsing::get_stdin()
{
	return (body);
}
