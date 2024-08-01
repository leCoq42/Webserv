#pragma once

#include "request.hpp"
#include <memory>
#include <vector>
#include "log.hpp"

class CGI {
	public:
		CGI();
		CGI(const std::shared_ptr<Request> &request, const std::filesystem::path &scriptPath, const std::string &interpreter);
		//CGI copy constructor
		//CGI = overload
		~CGI();

		const size_t		&get_contentLength() const;
		const std::string	&get_result() const;
		const int			&get_cgiFD() const;
		const bool			&isComplete() const;

		int			readCGIfd();

	private:
		void		parseCGI();
		void		executeScript();
		void		createArgs(std::vector<char *> &argv, std::string &path, std::string &args);
		void		init_envp();
		bool		add_to_envp(std::string name, std::string value, std::string prefix);
		bool		validate_key(std::string key);
		void		calculateContentLength();

		const std::shared_ptr<Request>	_request;
		std::filesystem::path			_scriptPath;
		std::string						_interpreter;
		std::vector<char *>				_cgiArgv;
		std::vector<std::string>		_cgiEnvp;
		std::string						_result;
		size_t							_contentLength;
		Log								_log;
		int								_cgiFD;
		bool							_complete;

	static const inline std::vector<std::string> metaVarNames = {
		"AUTH_TYPE",      "CONTENT_LENGTH",  "CONTENT_TYPE", "GATEWAY_INTERFACE",
		"PATH_INFO",      "PATH_TRANSLATED", "QUERY_STRING", "REMOTE_ADDR",
		"REMOTE_HOST",    "REMOTE_IDENT",    "REMOTE_USER",  "REQUEST_METHOD",
		"SCRIPT_NAME",    "SERVER_NAME",     "SERVER_PORT",  "SERVER_PROTOCOL",
		"SERVER_SOFTWARE"
	};
	
	static const inline std::vector<std::string> custom_var_prefixes = {"HTTP_"};
};
