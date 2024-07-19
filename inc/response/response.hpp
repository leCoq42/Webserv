#pragma once

#include "request.hpp"
#include "FileAcces.hpp"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <map>

enum class statusCode {
	OK = 200,
	CREATED = 201,
	ACCEPTED = 202,
	NO_CONTENT = 204,
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	SEE_OTHER = 303,
	NOT_MODIFIED = 304,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	PAYMENT_REQUIRED = 402,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	REQUEST_TIMEOUT = 408,
	LENGTH_REQUIRED = 411,
	PAYLOAD_TOO_LARGE = 413,
	URI_TOO_LONG = 414,
	UNSUPPORTED_MEDIA_TYPE = 415,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	BAD_GATEWAY = 502,
	SERVICE_UNAVAILABLE = 503,
	GATEWAY_TIMEOUT = 504
};

class Response {
public:
  Response(ServerStruct &config);
  Response(std::shared_ptr<Request> request, ServerStruct &config);
  Response(std::shared_ptr<Request> request, ServerStruct &config, std::string filename);
  ~Response();

  Response(const Response &src);
  Response &operator=(const Response &rhs);

  void			swap(Response &lhs);
  void			handleRequest(const std::shared_ptr<Request> &request);
  std::string	get_response();
  std::string	get_contentType();

  void printResponse();

private:
	std::shared_ptr<Request>	_request;
	std::string					_responseString;
	std::string					_contentType;
	std::string					_bufferFile;
	ServerStruct				&_config;
	FileAccess					_fileAccess;

	bool	handleGetRequest(const std::shared_ptr<Request> &request);
	bool	handlePostRequest(const std::shared_ptr<Request> &request);
	bool	handleDeleteRequest(const std::shared_ptr<Request> &request);
	void	handle_multipart();

	std::unordered_map<std::string, std::string>	get_args(std::string requestBody,
															   std::string contentType);
	std::vector<std::string>						split_multipart(std::string requestBody,
																	std::string boundary);
	std::string										extract_filename(const std::string &headers);
	statusCode										write_file(const std::string &path,
																const std::string &content, bool append);
	const std::string								readFileToBody(std::filesystem::path path);
	std::string										buildResponse(int status, const std::string &message,
																	const std::string &body, bool isCGI = false);

	static const inline std::unordered_map<std::string, std::string> contentTypes{
		{".html", "text/html"},
		{".txt", "text/plain"},

		{".jpeg", "image/jpeg"},
		{".jpg", "image/jpg"},
		{".png", "image/png"},
		{".gif", "image/gif"},

		{".cgi", "text/html"},
		{".php", "text/html"},
		{".py", "text/html"},

		{"x-www-form-urlencoded", "application/x-www-form-urlencoded"},
		{"form-data", "multipart/form-data"}};

	static const inline std::unordered_map<std::string, std::string> interpreters{
		{".cgi", ""},
		{".py", "/usr/bin/python3"},
		{".php", "/ usr / lib / cgi - bin / php"},
	};

	static const inline std::map<statusCode, std::string> statusCodeMap = {
		{statusCode::OK, "OK"},
		{statusCode::CREATED, "Created"},
		{statusCode::ACCEPTED, "Accepted"},
		{statusCode::NO_CONTENT, "No Content"},
		{statusCode::MOVED_PERMANENTLY, "Moved Permanently"},
		{statusCode::FOUND, "Found"},
		{statusCode::SEE_OTHER, "See Other"},
		{statusCode::NOT_MODIFIED, "Not Modified"},
		{statusCode::BAD_REQUEST, "Bad Request"},
		{statusCode::UNAUTHORIZED, "Unauthorized"},
		{statusCode::PAYMENT_REQUIRED, "Payment Required"},
		{statusCode::FORBIDDEN, "Forbidden"},
		{statusCode::NOT_FOUND, "Not Found"},
		{statusCode::METHOD_NOT_ALLOWED, "Method Not Allowed"},
		{statusCode::REQUEST_TIMEOUT, "Request Timeout"},
		{statusCode::LENGTH_REQUIRED, "Length Required"},
		{statusCode::PAYLOAD_TOO_LARGE, "Payload Too Large"},
		{statusCode::URI_TOO_LONG, "URI Too Long"},
		{statusCode::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
		{statusCode::INTERNAL_SERVER_ERROR, "Internal Server Error"},
		{statusCode::NOT_IMPLEMENTED, "Not Implemented"},
		{statusCode::BAD_GATEWAY, "Bad Gateway"},
		{statusCode::SERVICE_UNAVAILABLE, "Service Unavailable"},
		{statusCode::GATEWAY_TIMEOUT, "Gateway Timeout"}
	};
};
