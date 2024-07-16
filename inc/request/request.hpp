#pragma once

#include <cstddef>
#include <iostream>
#include <unordered_map>
#include <vector>

// doesn't seem to be the standard but it is an security issue not to check
// this, https://www.htmlhelp.com/faq/cgifaq.2.html
// https://datatracker.ietf.org/doc/html/rfc3875#section-4
//  std::string	specifiedCgiEnv[] = {}

enum class requestStatus {INCOMPLETE, STUCK, COMPLETE, FAILED};

class Request {
	public:
		Request();
		Request(const std::string str);
		Request(const Request &src);
		Request &operator=(const Request &rhs);
		void swap(Request &lhs);
		~Request();

		bool checkRequestValidity() const;

		void print_Request();

		const std::string	&get_rawRequest() const;
		const std::string	&get_requestMethod() const;
		const std::string	&get_uri() const;
		const std::string	get_referer() const;
		const std::string	get_contentType() const;
		const std::string	get_boundary() const;
		const std::unordered_map<std::string, std::string> &get_requestArgs() const;
		const std::string	&get_htmlVersion() const;
		const std::string	&get_connection() const;
		const std::string	&get_body() const;
		const std::unordered_map<std::string, std::string> &get_headers() const;
		const std::string	&get_bufferFile() const; //added
		size_t				parse_contentLen() const;
		const size_t		&get_contentLength() const; //added
		const bool			&get_keepAlive() const;
		const bool			&get_validity() const;
		const requestStatus	&get_requestStatus() const;
		void				printRequest() const;
		void				set_keepAlive(bool keepAlive); //added
		void				set_bufferFile(std::string buffer_file); //added
		void				set_contentLength(size_t contentLength); //added
		void				set_requestStatus(requestStatus);
		void appendToBody(std::string requestString);

	private:
		const std::string	_rawRequest;
		std::string			_requestMethod;
		std::string			_requestPath;
		std::string			_htmlVersion;
		std::unordered_map<std::string, std::string>	_requestArgs;
		std::unordered_map<std::string, std::string>	_headers;
		bool				_keepAlive;
		bool				_isValid;
		std::string			_body;
		std::unordered_map<std::string, std::string>	_cgiEnv;
		std::string			_bufferFile;//added
		size_t				_contentLength;
		bool				_chunked;
		requestStatus		_requestStatus;
		
		void		parseRequest();
		bool		parseRequestLine(const std::string &line);
		bool		parseRequestHeaders(std::istringstream &requestStream);
		std::string	parseRequestBody(const std::string &_rawRequest);
		void		extractCgiEnv();
		void		parseUrlArgs(const std::string uri);
		void		splitUrlArgs(std::string argStr);
};
