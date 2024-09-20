#pragma once

#include <unordered_map>
#include <filesystem>
#include <memory>
#include "log.hpp"

class Request {
public:
	Request(const std::string str, std::shared_ptr<Log> log);
	Request(const Request &src);
	Request &operator=(const Request &rhs);
	void swap(Request &lhs);
	~Request();

	void	print_Request();
	void	appendToBody(std::string requestString);
	std::string			_argStr;

	const std::string	&get_rawRequest() const;
	const std::string	&get_requestMethod() const;
	const std::string	&get_uri() const;
	const std::string	get_referer() const;
	const std::string	get_contentType() const;
	const std::string	get_boundary() const;
	const std::string	&get_htmlVersion() const;
	const std::string	&get_connection() const;
	const std::string	&get_body() const;
	size_t				parse_contentLen() const;
	const size_t		&get_contentLength() const;
	const bool			&isValid() const;
	const bool			&get_requestStatus() const;
	void				printRequest() const;
	const std::filesystem::path &get_requestPath() const;
	const std::unordered_map<std::string, std::string> &get_headers() const;
	const std::unordered_map<std::string, std::string> &get_requestArgs() const;
	void				set_requestStatus(bool status);
	std::string 		trim(const std::string &str, const std::string &tokens);

private:
	std::shared_ptr<Log>	_log;
	const std::string		_rawRequest;
	std::string				_requestMethod;
	std::filesystem::path	_requestPath;
	std::string				_htmlVersion;
	bool					_valid;
	std::string				_body;
	size_t					_contentLength;
	bool					_requestStatus;
	std::unordered_map<std::string, std::string>	_headers;
	std::unordered_map<std::string, std::string>	_requestArgs;
	
	void		parseRequest();
	bool		parseRequestLine(const std::string &line);
	bool		parseRequestHeaders(std::istringstream &requestStream);
	std::string	parseRequestBody(const std::string &_rawRequest);
	void		parseUrlArgs(const std::string uri);
	void		splitUrlArgs(std::string argStr);
	bool		checkRequestValidity() const;
};
