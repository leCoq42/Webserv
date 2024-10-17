#include "response.hpp"
#include "cgi.hpp"
#include "defines.hpp"
#include "dir_listing.hpp"
#include <algorithm>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

Response::Response(std::shared_ptr<Request> request, std::list<ServerStruct> *config, int port, std::shared_ptr<Log> log):
	_log(log), _request(request), _contentType(""), _body(""), _contentLength(0), _responseString(""),
	_fileAccess(config), _finalPath(""), _cgi(nullptr), _complete(false), _port(port)
{
	int return_code = 0;
	_finalPath = _request->get_requestPath();

	if (!_finalPath.empty() && _finalPath.string()[0] == '/')
		_finalPath = _finalPath.string().substr(1);

	_finalPath = _fileAccess.isFilePermissioned( _finalPath, return_code, port, _request->get_requestMethod(), _request->get_host());
	if (return_code == 301)
		buildResponse(static_cast<int>(return_code), redirect(_fileAccess.get_return()), false);
	else if (return_code) {
		_body = get_error_body(return_code, "Not Found.");
		buildResponse(static_cast<int>(return_code), "Not Found", false);
	}
	else
		handleRequest();
	#ifdef DEBUG
	printResponse();
	#endif
}

Response::Response(int error_code, std::string error_description, std::list<ServerStruct> *config, int port, std::shared_ptr<Log> log) :
	_log(log), _request(nullptr), _contentType("text/html"), _body(""), _contentLength(0), _responseString(""),
	_fileAccess(config), _finalPath(""), _cgi(nullptr), _complete(false), _port(port)
{
	_finalPath = _fileAccess.isFilePermissioned( _finalPath, error_code, _port, "GET", "");
	_body = get_error_body(error_code, error_description);
	buildResponse(error_code, error_description, false);
	#ifdef DEBUG
	printResponse();
	#endif

}

Response::~Response() {}

Response::Response(const Response &src):
	_log(src._log), _request(src._request), _contentType(src._contentType), _body(src._body), _contentLength(src._contentLength),
	_responseString(src._responseString), _fileAccess(src._fileAccess),
	_finalPath(src._finalPath), _cgi(src._cgi), _complete(src._complete)
{}

Response	&Response::operator=(const Response &rhs)
{
	Response temp(rhs);
	temp.swap(*this);
	return *this;
}

void	Response::swap(Response &lhs)
{
	std::swap(_log, lhs._log);
	std::swap(_request, lhs._request);
	std::swap(_contentType, lhs._contentType);
	std::swap(_body, lhs._body);
	std::swap(_contentLength, lhs._contentLength);
	std::swap(_responseString, lhs._responseString);
	std::swap(_fileAccess, lhs._fileAccess);
	std::swap(_finalPath, lhs._finalPath);
	std::swap(_cgi, lhs._cgi);
	std::swap(_complete, lhs._complete);
}

std::string	Response::get_error_body(int error_code, std::string error_description)
{
	std::string				error_body;
	std::filesystem::path	error_page;

	error_page = _fileAccess.getErrorPage(error_code);
	if (!error_page.empty())
		error_body = readFileToBody(error_page);
	else
		error_body = standard_error(error_code, error_description);
	return (error_body);
}

void	Response::handleRequest()
{
	if (_request && !_request->isValid()) {
		_body = get_error_body(static_cast<int>(statusCode::BAD_REQUEST), "Bad Request.");
		buildResponse(static_cast<int>(statusCode::BAD_REQUEST), "Bad Request");
		return;
	}
	std::string request_method = _request->get_requestMethod();
	try {
		if (request_method == "GET" && _fileAccess.allowedMethod("GET"))
			handleGetRequest();
		else if (request_method == "POST" && _fileAccess.allowedMethod("POST"))
			handlePostRequest();
		else if (request_method == "DELETE" && _fileAccess.allowedMethod("DELETE"))
			handleDeleteRequest();
		else
		{
			_body = get_error_body(static_cast<int>(statusCode::METHOD_NOT_ALLOWED), "Method not allowed.");
			buildResponse(static_cast<int>(statusCode::METHOD_NOT_ALLOWED),
						"Method Not Allowed", false);
		}
	}
	catch (const std::exception &e) {
		_body = get_error_body(static_cast<int>(statusCode::INTERNAL_SERVER_ERROR), "Internal Server Error.");
		buildResponse(static_cast<int>(statusCode::INTERNAL_SERVER_ERROR),
					"Internal Server Error", false);
	}
}

bool	Response::handleGetRequest()
{
	std::stringstream buffer;

	int status_code;

	status_code = 0;
	if (!_finalPath.empty() && _finalPath.has_extension())
	{
		std::unordered_map<std::string, std::string>::const_iterator it =
			contentTypes.find(_finalPath.extension());
		if (it == contentTypes.end())
		{
			_body = get_error_body(static_cast<int>(statusCode::OK), "Unsupported Media Type");
			buildResponse(static_cast<int>(statusCode::UNSUPPORTED_MEDIA_TYPE),
				  "Unsupported Media Type", false);
			return false;
		}
		_contentType = it->second;
		if (interpreters.find(_finalPath.extension()) == interpreters.end())
		{
			_body = readFileToBody(_finalPath);
			if (_body.empty())
			{
				_body = get_error_body(404, "File not found.");
				buildResponse(static_cast<int>(statusCode::NOT_FOUND), "Not Found", false);
				return true;
			}
		}
		else
		{
			_cgi = std::make_shared<CGI>(_request, _finalPath, interpreters.at(_finalPath.extension()), _log);
			if (_cgi->isComplete()) {
				_body = _cgi->get_result();
				_contentLength = _cgi->get_contentLength();
				buildResponse(static_cast<int>(statusCode::OK), "OK", true);
			}
			return true;
		}
	}
	else
		_body = list_dir(_finalPath, _request->get_requestPath(), _request->get_referer(), status_code);
	if (!status_code)
		buildResponse(static_cast<int>(statusCode::OK), "OK", false);
	else
	{
		_body = get_error_body(404, "File not found.");
		buildResponse(status_code, "Not Found", false);
	}
	return true;
}

bool	Response::handlePostRequest()
{
	std::string requestBody = _request->get_body();
	std::string requestContentType = _request->get_contentType();
	bool isCGI = false;

	if (!_finalPath.empty() && _finalPath.string()[0] == '/')
		_finalPath = _finalPath.string().substr(1);

	if (_finalPath.has_extension()) {
		if (interpreters.find(_finalPath.extension()) != interpreters.end()) {
			isCGI = true;
			_cgi = std::make_shared<CGI>(_request, _finalPath, interpreters.at(_finalPath.extension()), _log);
			if (_cgi->isComplete()) {
				_body = _cgi->get_result();
				_contentLength = _cgi->get_contentLength();
				buildResponse(static_cast<int>(statusCode::OK), "OK", isCGI);
			}
			return true;
		}
		else {
			_body = get_error_body(static_cast<int>(statusCode::NO_CONTENT), "No Content.");
			buildResponse(static_cast<int>(statusCode::NO_CONTENT), "No Content", "");
			return true;
		}
	}
	if (requestContentType == "multipart/form-data") {
		handle_multipart();
		return true;
	}
	buildResponse(static_cast<int>(statusCode::OK), "OK", isCGI);
	return true;
}

bool	Response::handleDeleteRequest()
{
	if (_fileAccess.is_deleteable(_finalPath))
	{
		if (std::remove(_finalPath.c_str()))
		{
			_body = get_error_body(static_cast<int>(204), "Delete.");
			buildResponse(static_cast<int>(204), "Delete.", "");
			return true;
		}
	}
	_body = get_error_body(static_cast<int>(204), "Delete.");
	buildResponse(static_cast<int>(204), "Delete.", "");
	return false;
}

const std::string	Response::readFileToBody(std::filesystem::path path)
{
	std::string body;
	std::ifstream infile(path, std::ios::binary);

	if (!infile) {
		_log->logError(std::string("Error, invalid path: " + path.string()));
		return "";
	}
	infile.seekg(0, std::ios::end);
	body.resize(infile.tellg());
	infile.seekg(0, std::ios::beg);
	infile.read(&body[0], body.size());
	infile.close();
	return body;
}

void	Response::handle_multipart()
{
	statusCode	status = statusCode::OK;
	std::string	requestBody = _request->get_body();
	std::string	boundary = _request->get_boundary();
	std::string	filename = "";
	bool		append = false;
	int			status_code = 0;

	std::vector<std::string> parts = split_multipart(requestBody, boundary);
	for (const std::string &part: parts) {
		if (part.empty())
			continue;

		size_t header_end = part.find(CRLFCRLF);
		if (header_end == std::string::npos)
			continue;

		std::string headers = part.substr(0, header_end);
		std::string content = part.substr(header_end + 4);
		
		std::transform(headers.begin(), headers.end(), headers.begin(), [](unsigned char c){ return std::tolower(c);} );

		size_t contentType = headers.find("content-type:");
		if (contentType == std::string::npos)
			continue;

		filename = extract_filename(headers);
		status = write_file(      _finalPath.string() + "/" + filename, content, append);
		if (status != statusCode::OK)
			break;
		append = true;
		#ifdef DEBUG
		std::cout << "File Upload success!" << std::endl;
		std::cout << MSG_BORDER << MSG_BORDER << std::endl;
		#endif
		if (status == statusCode::OK)
			_body = list_dir(_finalPath, _request->get_requestPath(), _request->get_referer(), status_code);
		else
			_body = get_error_body(static_cast<int>(status), "File not found.");
	}
	buildResponse(static_cast<int>(status), statusCodeMap.at(status), false);
}

std::vector<std::string>	Response::split_multipart(std::string requestBody, std::string boundary)
{
	std::vector<std::string> parts;
	std::string fullBoundary = "--" + boundary;
	size_t pos = 0;

	while (pos < requestBody.size()) {
		size_t start = requestBody.find(fullBoundary, pos);
		if (start == std::string::npos)
			break;
		size_t end = requestBody.find(fullBoundary, start + fullBoundary.length());
		if (end  == std::string::npos)
			end = requestBody.size();
		parts.push_back(requestBody.substr(start + fullBoundary.length(), end - start - fullBoundary.length() - 2));
		pos = end;
	}
	return parts;
}

std::string	Response::extract_filename(const std::string &headers)
{
	size_t filename_pos = headers.find("filename=\"");

	if (filename_pos == std::string::npos)
				return "temp.txt";
	size_t filename_end = headers.find("\"", filename_pos + 10);
	return headers.substr(filename_pos + 10, filename_end - filename_pos - 10);
}

statusCode	Response::write_file(const std::string &path, const std::string &content, bool append)
{
	std::ofstream file;
	if (append)
		file.open(path, std::ios::binary | std::ios::app);
	else
		file.open(path, std::ios::binary);
	if (file.is_open()) {
		file.write(content.c_str(), content.length());
		file.close();
		return statusCode::OK;
	}
	else
		return statusCode::INTERNAL_SERVER_ERROR;
}

int	Response::continue_cgi()
{
	if (_cgi->isComplete() == true) {
		_body = _cgi->get_result();
		_contentLength = _cgi->get_contentLength();
		buildResponse(static_cast<int>(statusCode::OK), "OK", true);
		return (1);
	}
	if (_cgi->get_pollfdWrite().revents & POLLOUT && _cgi->writeCGIfd())
		;
	if (_cgi->get_pollfdRead().revents & POLLIN || _cgi->get_pollfdRead().revents == POLLHUP || _cgi->get_pollfdRead().revents == POLLNVAL){
		// if (_cgi->get_pollfdRead().revents == POLLHUP || _cgi->get_pollfdRead().revents == POLLNVAL)
		// 	_cgi->readCGIfd();
		if (_cgi->readCGIfd()) {
			std::cerr << "cgi reading error" << std::endl;
			_body = get_error_body(static_cast<int>(statusCode::INTERNAL_SERVER_ERROR), "Internal Server Error.");
			buildResponse(static_cast<int>(statusCode::INTERNAL_SERVER_ERROR), "Internal Server Error", false);
		}
		return (1);
	}
	if (_cgi->isComplete() == true) {
		_body = _cgi->get_result();
		_contentLength = _cgi->get_contentLength();
		buildResponse(static_cast<int>(statusCode::OK), "OK", true);
		return (1);
	}
	return (0);
}

void	Response::buildResponse(int status, const std::string &message, bool isCGI)
{
	_responseString.append("HTTP/1.1 " + std::to_string(status) + " " + message +
							CRLF);
	if (isCGI) {
		_responseString.append("Content-Length: " + std::to_string(_contentLength) +
							CRLF);
		_responseString.append(_body);
	}
	else {
		if (_body.empty())
		{
			_complete = true; // this was missing
			return;
		}
		else {
			_responseString.append("Content-Length: " + std::to_string(_body.length()) +
						  CRLF);
			_responseString.append("Content-Type: " + get_contentType() + CRLF);
			_responseString.append(CRLF + _body);
		}
	}
	_complete = true;
}

const std::string	&Response::get_response() const { return _responseString; }
const std::string	&Response::get_contentType() const { return _contentType; }
bool 				Response::isComplete() const { return _complete; }
const std::shared_ptr<CGI>	Response::get_cgi() {return _cgi;};

void Response::printResponse() {
	std::cout << MSG_BORDER << "[Response]" << MSG_BORDER << std::endl;
	std::cout << _responseString << std::endl;
}
