#pragma once

#include "request.hpp"
#include <memory>

class CGI {
	public:
		CGI();
		CGI(const std::shared_ptr<Request> &request, const std::filesystem::path &scriptPath);
		~CGI();

		void parseCGI();
		void createArgs(std::vector<char *> &argv, std::string &path, std::string &args);

		std::string executeCGI();

		// void createEnv(std::vector<char *> &envp);
		void init_envp();
		bool add_to_envp(std::string name, std::string value, std::string prefix);
		bool validate_key(std::string key);

	private:
		const std::shared_ptr<Request>	_request;
		std::filesystem::path			_scriptPath;
		std::vector<std::string>		_cgiEnvp;
		std::vector<std::string>		_cgiArgv;

	static const inline std::vector<std::string> metaVarNames = {
		"AUTH_TYPE",      "CONTENT_LENGTH",  "CONTENT_TYPE", "GATEWAY_INTERFACE",
		"PATH_INFO",      "PATH_TRANSLATED", "QUERY_STRING", "REMOTE_ADDR",
		"REMOTE_HOST",    "REMOTE_IDENT",    "REMOTE_USER",  "REQUEST_METHOD",
		"SCRIPT_NAME",    "SERVER_NAME",     "SERVER_PORT",  "SERVER_PROTOCOL",
		"SERVER_SOFTWARE"
	};
	
	static const inline std::vector<std::string> custom_var_prefixes = {};
};
