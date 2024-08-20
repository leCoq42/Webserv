#include "request.hpp"
#include "defines.hpp"
#include "stringUtils.hpp"
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <sstream>
#include <filesystem>


Request::Request() :
	_rawRequest(""), _requestMethod(""), _requestPath(""),
	_htmlVersion(""), _valid(0), _body(""), _contentLength(0), _requestStatus(false)
{}

Request::Request(const std::string rawStr) :
	_rawRequest(rawStr), _requestMethod(""), _requestPath(""),
	_htmlVersion(""), _valid(0), _body(""), _contentLength(0), _requestStatus(false)
{
	parseRequest();

	if (_body.length() != _contentLength) {
		_requestStatus = false;
		#ifdef DEBUG
		std::cout << "Incomplete Request >>>>>>>>>>>" << std::endl;
		printRequest();
		#endif
	}
	else{
		_requestStatus = true;
		#ifdef DEBUG
		std::cout << "Complete request >>>>>>>>>>>>" << std::endl;
		printRequest();
		#endif
	}
}

Request::~Request() {}

Request::Request(const Request &src) :
	_rawRequest(src._rawRequest), _requestMethod(src._requestMethod),
	_requestPath(src._requestPath), _htmlVersion(src._htmlVersion),
	_valid(src._valid), _body(src._body), _contentLength(src._contentLength),
	_requestStatus(src._requestStatus), _headers(src._headers),
	_requestArgs(src._requestArgs)
{}

Request	&Request::operator=(const Request &rhs) {
	Request temp(rhs);
	temp.swap(*this);
	return *this;
}

void	Request::swap(Request &lhs) {
	std::swap(_requestMethod, lhs._requestMethod);
	std::swap(_requestPath, lhs._requestPath);
	std::swap(_htmlVersion, lhs._htmlVersion);
	std::swap(_valid, lhs._valid);
	std::swap(_body, lhs._body);
	std::swap(_contentLength, lhs._contentLength);
	std::swap(_requestStatus, lhs._requestStatus);
	std::swap(_headers, lhs._headers);
	std::swap(_requestArgs, lhs._requestArgs);
}

void	Request::parseRequest()
{
	std::string line;
	std::string headerKey;
	std::string headerValue;

	if (_rawRequest.empty()) {
		_valid = false;
		return;
	}

	std::istringstream requestStream(_rawRequest);
	std::getline(requestStream, line, '\r');
	requestStream.get();

	if (!parseRequestLine(line)) {
		_valid = false;
		return;
	}

	_requestPath = trim(_requestPath, "/");
	if (_requestMethod == "GET")
		parseUrlArgs(_requestPath);
	if (!parseRequestHeaders(requestStream)) {
		_valid = false;
		return;
	}

	_contentLength = parse_contentLen();
	_body = parseRequestBody(_rawRequest);
	_valid = checkRequestValidity();
	return;
}

void	Request::parseUrlArgs(const std::string uri)
{
	size_t pos;
	std::unordered_map<std::string, std::string> args;
	std::string argStr;

	pos = uri.find("?");
	if (pos != std::string::npos) {
		_requestPath = uri.substr(0, pos);
		if (pos + 1)
		{
			argStr = uri.substr(pos + 1);
			splitUrlArgs(argStr);
		}
	}
}

void	Request::splitUrlArgs(std::string argStr)
{
	std::string arg;
	std::istringstream argStream(argStr);

	while (std::getline(argStream, arg, '&'))
	{
		size_t pos = arg.find('=');
		if (pos != std::string::npos)
		{
			std::string key = arg.substr(0, pos);
			std::string value = arg.substr(pos + 1);
			_requestArgs[key] = value;
		}
	}
}


bool	Request::parseRequestLine(const std::string &line)
{
	std::istringstream lineStream(line);
	if (!(lineStream >> _requestMethod >> _requestPath >> _htmlVersion))
		return false;
	return true;
}

bool	Request::parseRequestHeaders(std::istringstream &requestStream)
{
	std::string line;
	size_t pos;
	size_t headerPos;
	std::string headerKey;
	std::string headerValue;

	while (std::getline(requestStream, line) && !line.empty() && line != "\r") {
		pos = line.find(":");
		if (pos != std::string::npos) {
		headerPos = pos + 1;
		while (line[headerPos] == ' ')
			headerPos++;

		headerKey = line.substr(0, pos);
		for (auto &c : headerKey)
			c = tolower(c);

		headerValue =
			line.substr(headerPos, line.find_last_not_of("/r") - headerPos);
		_headers[headerKey] = headerValue;
		}
	}
	return true;
}

std::string	Request::parseRequestBody(const std::string &_rawRequest)
{
	size_t body_start;
	std::string body;

	body_start = _rawRequest.find(CRLFCRLF);
	if (body_start == std::string::npos)
		return "";
	body = _rawRequest.substr(body_start + 4, _contentLength);
	return body;
}

void	Request::appendToBody(std::string part)
{
	_body.append(part);
	if (_body.length() == _contentLength) {
		_requestStatus = true;
	}
}

bool	Request::checkRequestValidity() const
{
	if (_requestMethod.empty() || _htmlVersion.empty())
		return false;
	if (_requestMethod != "GET" && _requestMethod != "POST" &&
		_requestMethod != "DELETE")
		return false;
	if (_htmlVersion != "HTTP/1.0" && _htmlVersion != "HTTP/1.1" &&
		_htmlVersion != "HTTP/2.0")
		return false;
	if (_htmlVersion != "HTTP/1.0" && _headers.find("host") == _headers.end())
		return false;
	return true;
}

const std::string	&Request::get_rawRequest() const { return _rawRequest; }

const std::string	&Request::get_requestMethod() const { return _requestMethod; }

const std::string Request::get_referer() const
{
	auto referer = _headers.find("referer");
	if (referer == _headers.end())
		return "";
	return referer->second;
}

const std::string	Request::get_contentType() const
{
	auto res = _headers.find("content-type");
	if (res == _headers.end())
		return "";
	std::string contentType = res->second;
	size_t pos = contentType.find(";");
	if (pos != std::string::npos) {
		contentType = contentType.substr(0, pos);
	}
	return contentType;
}

size_t	Request::parse_contentLen() const
{
	auto contentLenStr = _headers.find("content-length");
	if (contentLenStr == _headers.end())
		return 0;

	try {
		size_t contentLen = std::stoul(contentLenStr->second);
		return contentLen;
	} catch (const std::invalid_argument &e) {
		return 0;
	} catch (const std::out_of_range &e) {
		return 0;
	}
}

const std::string	Request::get_boundary() const {
	std::string ret;

	auto boundary = _headers.find("content-type");
	if (boundary != _headers.end()) {
		ret = boundary->second.substr(boundary->second.find("boundary=") + 9);
	} else {
		ret = "";
	}
	return ret;
}

const std::string	&Request::get_body() const { return _body; }

const std::filesystem::path	&Request::get_requestPath() const { return _requestPath; }

const size_t	&Request::get_contentLength() const { return (_contentLength); }//added

const std::string	&Request::get_htmlVersion() const { return _htmlVersion; }

const std::unordered_map<std::string, std::string>	&Request::get_requestArgs() const {
  return _requestArgs;
}

const std::unordered_map<std::string, std::string>	&Request::get_headers() const {
	return _headers;
}

const bool	&Request::isValid() const { return _valid; }

const bool	&Request::get_requestStatus() const { return _requestStatus; }

void	Request::set_requestStatus(bool status) { _requestStatus = status; }

void	Request::printRequest() const {
	std::cout << MSG_BORDER << "[Complete Request:]" << MSG_BORDER << std::endl;
	std::cout << get_rawRequest() << std::endl;

	#ifdef DEBUG
	std::cout << MSG_BORDER << "[Parsed Request]" << MSG_BORDER << std::endl;
	std::cout << "request method: " << get_requestMethod() << std::endl;
	std::cout << "request path: " << get_requestPath() << std::endl;
	std::cout << "html version: " << get_htmlVersion() << std::endl;

	std::cout << "<URI Args>" << std::endl;
	for (auto it : _requestArgs) {
		std::cout << "Key: " << it.first << " Value: " << it.second << std::endl;
	}

	std::cout << "<Headers>" << std::endl;
	std::unordered_map<std::string, std::string> headers = get_headers();
	for (auto it : headers) {
		std::cout << it.first << ": " << it.second << std::endl;
	}

	if (get_requestMethod() == "POST") {
		std::cout << "<POST Header Getters>" << std::endl;
		std::cout << "Referer: " << get_referer() << std::endl;
		std::cout << "ContentType: " << get_contentType() << std::endl;
		std::cout << "Boundary: " << get_boundary() << std::endl;
		std::cout << "ContentLen: " << get_contentLength() << std::endl;
	}

	std::cout << "<Body>" << std::endl;
	std::string body = get_body();
	(body.empty()) ? std::cout << "Empty Body" << std::endl
					: std::cout << body << std::endl;

	std::cout << "<Request Validity>" << std::endl;
	(checkRequestValidity() == true) ? (std::cout << "Valid!" << std::endl)
									: (std::cout << "Invalid!" << std::endl);
	#endif
}
