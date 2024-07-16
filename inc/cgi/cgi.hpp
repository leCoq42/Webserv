#pragma once

#include "request.hpp"
#include <memory>

class CGI {
	public:
		CGI();
		CGI(const std::shared_ptr<Request> &request);
		~CGI();

		void createArgs(std::vector<char *> &argv, std::string &path, std::string &args);

		std::string executeCGI(const std::string &path, const std::string &args,
								std::shared_ptr<Request> _request, std::string interpreter);

		// void createEnv(std::vector<char *> &envp);
		bool validate_key(std::string key, std::vector<std::string> customizable_variables_names);
		bool add_to_envp(std::string name, std::string value, std::string additive);
		bool add_to_argv(std::string name, std::string value, std::string additive);

	private:
		const std::shared_ptr<Request>					_request;
		std::unordered_map<std::string, std::string>	_cgiEnv;
		std::vector<std::string>						_cgiArgv;

	static const inline std::vector<std::string> metaVarNames = {
		"AUTH_TYPE",      "CONTENT_LENGTH",  "CONTENT_TYPE", "GATEWAY_INTERFACE",
		"PATH_INFO",      "PATH_TRANSLATED", "QUERY_STRING", "REMOTE_ADDR",
		"REMOTE_HOST",    "REMOTE_IDENT",    "REMOTE_USER",  "REQUEST_METHOD",
		"SCRIPT_NAME",    "SERVER_NAME",     "SERVER_PORT",  "SERVER_PROTOCOL",
		"SERVER_SOFTWARE"
	};
};
