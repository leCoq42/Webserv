#include "cgiParsing.hpp"
#include "cgi.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

std::vector<std::string> meta_variables_names = {
    "AUTH_TYPE",      "CONTENT_LENGTH",  "CONTENT_TYPE", "GATEWAY_INTERFACE",
    "PATH_INFO",      "PATH_TRANSLATED", "QUERY_STRING", "REMOTE_ADDR",
    "REMOTE_HOST",    "REMOTE_IDENT",    "REMOTE_USER",  "REQUEST_METHOD",
    "SCRIPT_NAME",    "SERVER_NAME",     "SERVER_PORT",  "SERVER_PROTOCOL",
    "SERVER_SOFTWARE"};

//started/unfinished function for editing the body for stdin, for example trimming boundaries and or removing additional headers
bool CgiParsing::dismantle_body(std::string body, std::string boundary) {
	std::string contentDisposition;
	std::string contentType;

	if (boundary.empty())
		;
	// std::cout << "BOUNDARY:" << boundary << std::endl;
	// std::cout << "BODY:" << std::endl << body << std::endl;
	// std::cout << "End of BODY." << std::endl;
	// contentDisposition = body.substr(body.find("Content-Disposition: "),
	// body.substr(body.find("Content-Disposition: ")).find("\n")); std::cout <<
	// "CONTENT-DISPOSITION:" << contentDisposition << std::endl; contentType =
	// body.substr(body.find("Content-Type: "),
	// body.substr(body.find("Content-Type: ")).find("\n")); std::cout <<
	// "CONTENT-TYPE:" << contentType << std::endl; body = body.find("\r\n\r\n");
	// std::cout << "body:" << body << std::endl;
	this->_body = body;
	return (false);
}

//$_GET['name'] = hoi php file is argv name=hoi :: $_SERVER['name'] = doei php
// file is env var name=doei :: educated guess is that _POST['var'] = something
// goes into the stdin var=something
CgiParsing::CgiParsing(
    std::unordered_map<std::string, std::string> headers, char **environ,
    std::shared_ptr<Request> _request, const std::string &path,
    const std::string &interpreter) // ServerStruct &serverinfo
{
  // int i;

	//Prefixes that are acceptable to be put into envp, see rfc documentation on specifications of what is accepted
  customizable_variables_names.push_back("X_");
  customizable_variables_names.push_back("");

  // i = -1;

  // MAGIC env variables for PHP REDIRECT_STATUS=true, PHP gives an security error if these headers ar not included
  // SCRIPT_FILENAME=/var/www/... REQUEST_METHOD=POST GATEWAY_INTERFACE=CGI/1.1
  if (!_request->get_requestMethod().compare("POST")) {
    add_to_envpp("REQUEST_METHOD", _request->get_requestMethod(), "");
    add_to_envpp("GATEWAY_INTERFACE", "CGI/1.1", "");
  }
  add_to_envpp("REDIRECT_STATUS", "true", "");
  add_to_envpp("SCRIPT_FILENAME", path, "");

	if (environ) //environ is passed to function, but dont think it's desired to pass it to execve
	{
	// while (*(environ + ++i))
	// 	add_to_envpp(((std::string)*(environ + i)).substr(0,
	// ((std::string)*(environ + i)).find("=")), ((std::string)*(environ +
	// i)).substr(((std::string)*(environ + i)).find("=") + 1), "");
		;
	}

	//try to add all headers to envp, gets checked over what is permissioned
  for (const auto &[key, value] : headers)
    add_to_envpp(key, value, "");

  // adding variables to argv 
  if (interpreter.compare(""))
    add_to_uri(interpreter, "", "");
  add_to_uri(path, "", "");
  for (const auto &var : _request->get_requestArgs())
    add_to_uri(var, "", "");

	//Disect body if needed
  dismantle_body(_request->get_body(), _request->get_boundary());
}

CgiParsing::~CgiParsing(void) {}

bool validate_key(std::string key,
                  std::vector<std::string> customizable_variables_names) {
  int i;

  for (auto &c : key)
    c = toupper(c);
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

// adds variable to envpp if permissed. additive is a specified prefix
bool CgiParsing::add_to_envpp(std::string name, std::string value,
                              std::string additive) {
  std::string temp;
  if (validate_key(additive + name, customizable_variables_names)) {
    temp = additive + name;
    for (auto &c : temp)
      c = toupper(c);
    if (value.compare(""))
      temp += "=" + value;
    std::replace(temp.begin(), temp.end(), '-', '_');
    _metaVars.push_back(temp); // uri[name] = value;
    return true;
  }
  return false;
}

// adds variable to argv. additive is specified prefix. commented part is to put rejected env variables in the argv
bool CgiParsing::add_to_uri(std::string name, std::string value,
                            std::string additive) {
  std::string temp;
  // if (!validate_key(additive + name, customizable_variables_names))
  // {
  temp = additive + name;
  if (value.compare(""))
    temp += "=" + value;
  _uri.push_back(temp); // uri[name] = value;
  return true;
  // }
  // return false;
}

std::vector<std::string> &CgiParsing::get_argv() { return (_uri); }

std::vector<std::string> &CgiParsing::get_envp() { return (_metaVars); }

std::string &CgiParsing::get_stdin() { return (_body); }
