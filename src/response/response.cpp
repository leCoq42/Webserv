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

Response::Response(std::list<ServerStruct> *config) : _request(nullptr), _responseString(""), _config(config), _fileAccess(config) {}

Response::Response(std::shared_ptr<Request> request, ServerStruct &config)
    : _request(request), _contentType(""), _body(""), _contentLength(0), _responseString(""),
	_bufferFile(""), _config(config), _fileAccess(config), _complete(true)
{
	int return_code = 0;

	_finalPath = _request->get_requestPath();

	if (!_finalPath.empty() && _finalPath.string()[0] == '/')
		_finalPath = _finalPath.string().substr(1);

	_finalPath = _fileAccess.isFilePermissioned( _finalPath, return_code, port);
	if (return_code) {
		std::cout << return_code << "Path error:" << _finalPath << std::endl;
		_finalPath = _fileAccess.getErrorPage(return_code); // wrong place
		buildResponse(static_cast<int>(return_code), "Not Found", "");
	}
	else {
		// std::cout << "File Access Path -> " << _finalPath << std::endl;
		handleRequest(request);
	}
	#ifdef DEBUG
	printResponse();
	#endif

}

// Response::Response(std::shared_ptr<Request> request, ServerStruct &config, std::string filename)
//     : _request(request), _responseString(""), _contentType(""), _bufferFile(filename), _config(config), _fileAccess(config) {
// 	int return_code = 0;
//
// 	std::filesystem::path newPath = _fileAccess.isFilePermissioned(request->get_requestPath(), return_code);
// 	if (return_code) {
// 		std::cout << "Return code: " << return_code << std::endl;
// 		std::cout << "New Path:" << _request->get_requestPath() << std::endl;
// 		_request->set_requestPath (_fileAccess.getErrorPage(return_code)); // wrong place
// 		buildResponse(static_cast<int>(return_code), "Not Found", "");
// 	}
// 	else {
// 		_request->set_requestPath(newPath);
// 		std::cout << "New Path:" << _request->get_requestPath() << std::endl;
// 		handleRequest(request);
// 	}
// 	#ifdef DEBUG
// 	printResponse();
// 	#endif
// }

Response::~Response() {}

Response::Response(const Response &src)
    : _request(src._request), _contentType(src._contentType), _body(src._body), _contentLength(src._contentLength),
	_responseString(src._responseString), _bufferFile(src._bufferFile), _config(src._config),
	_fileAccess(src._config), _finalPath(src._finalPath), _complete(src._complete)
{}

Response &Response::operator=(const Response &rhs)
{
	Response temp(rhs);
	temp.swap(*this);
	return *this;
}

void Response::swap(Response &lhs)
{
	std::swap(_request, lhs._request);
	std::swap(_contentType, lhs._contentType);
	std::swap(_responseString, lhs._responseString);
}

void Response::handleRequest(const std::shared_ptr<Request> &request)
{
	std::string request_method = request->get_requestMethod();
	try {
		if (request_method == "GET" && _fileAccess.allowedMethod("GET"))
			handleGetRequest(request);
		else if (request_method == "POST" && _fileAccess.allowedMethod("POST"))
			handlePostRequest(request);
		else if (request_method == "DELETE" && _fileAccess.allowedMethod("DELETE"))
			handleDeleteRequest(request);
		else
			buildResponse(static_cast<int>(statusCode::METHOD_NOT_ALLOWED),
						"Method Not Allowed", false);
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		buildResponse(static_cast<int>(statusCode::INTERNAL_SERVER_ERROR),
					"Internal Server Error", false);
	}
}

bool Response::handleGetRequest(const std::shared_ptr<Request> &request) {
	std::stringstream buffer;
	bool isCGI = false;

	if (!_finalPath.empty() && _finalPath.has_extension())
	{
		std::unordered_map<std::string, std::string>::const_iterator it =
			contentTypes.find(_finalPath.extension());
		if (it == contentTypes.end())
		{
				buildResponse(static_cast<int>(statusCode::UNSUPPORTED_MEDIA_TYPE),
				  "Unsupported Media Type", false);
			return false;
		}
		_contentType = it->second;

		if (interpreters.find(_finalPath.extension()) == interpreters.end())
		{
			_body = readFileToBody(_finalPath);
			if (_body.empty())
				return false;
		}
		else
		{
			isCGI = true;
			_cgi = std::make_unique<CGI>(_request, _finalPath, interpreters.at(_finalPath.extension()));
			_complete = _cgi->isComplete();
			if (_complete == true) {
				_body = _cgi->get_result();
				_contentLength = _cgi->get_contentLength();
				buildResponse(static_cast<int>(statusCode::OK), "OK", isCGI); // when cgi double padded?
			}
			return true;
		}
	}
	else
		_body = list_dir(_finalPath, request->get_requestPath(), request->get_referer());
	buildResponse(static_cast<int>(statusCode::OK), "OK", isCGI); // when cgi double padded?
	return true;
}

bool Response::handlePostRequest(const std::shared_ptr<Request> &request)
{
	std::string requestBody = request->get_body();
	std::string requestContentType = request->get_contentType();
	bool isCGI = false;

	if (!_finalPath.empty() && _finalPath.string()[0] == '/')
		_finalPath = _finalPath.string().substr(1);

	std::cout << "_finalPath:" << _finalPath << std::endl;
	if (_finalPath.has_extension()) {
		if (interpreters.find(_finalPath.extension()) != interpreters.end()) {
			isCGI = true;
			_cgi = std::make_unique<CGI>(_request, _finalPath, interpreters.at(_finalPath.extension()));
			_complete = _cgi->isComplete();
			if (_complete == true) {
				_body = _cgi->get_result();
				_contentLength = _cgi->get_contentLength();
				buildResponse(static_cast<int>(statusCode::OK), "OK", isCGI); // when cgi double padded?
			}
			return true;
		}
		else {
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

bool Response::handleDeleteRequest(const std::shared_ptr<Request> &request)
{
	std::filesystem::path Path = request->get_requestPath();

	buildResponse(static_cast<int>(statusCode::OK), "OK", "");
	return true;
}

const std::string	Response::readFileToBody(std::filesystem::path path)
{
	std::stringstream buffer;
	std::string body;
	std::ifstream file( path, std::ios::binary);

	if (!file) {
		std::cout << "Error, invalid path: " << path << std::endl;
		return "";
	}
	buffer << file.rdbuf();
	body = buffer.str();
	return body;
}

void Response::handle_multipart()
{
	statusCode	status = statusCode::OK;
	std::string	requestBody = _request->get_body();
	std::string	boundary = _request->get_boundary();
	std::string	filename = "";
	bool		append = false;

	if (_bufferFile.empty())
	{
		std::vector<std::string> parts = split_multipart(requestBody, boundary);
		#ifdef DEBUG
		std::cout << MSG_BORDER << "[MULTIPART REQUEST]" << MSG_BORDER << std::endl;
		std::cout << "Number of Parts: " << parts.size() << std::endl;
		#endif
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
			if (contentType == std::string::npos) {
				continue;
			}

			filename = extract_filename(headers);
			status = write_file(      "html/uploads/" + filename, content, append); //TODO:make customizable via config
			if (status != statusCode::OK)
				break;
			append = true;
			#ifdef DEBUG
			std::cout << "File Upload success!" << std::endl;
			std::cout << MSG_BORDER << MSG_BORDER << std::endl;
			#endif // DEBUG
		}
		if (status == statusCode::OK)
			_body = readFileToBody("html/upload_success.html");
		else
			_body = readFileToBody("html/standard_404.html");
	}
	else {//TODO should be able to run cgi as well
		status = statusCode::OK;
	}
	buildResponse(static_cast<int>(status), statusCodeMap.at(status), false);
}

std::vector<std::string> Response::split_multipart(std::string requestBody, std::string boundary)
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

std::string Response::extract_filename(const std::string &headers) {
	size_t filename_pos = headers.find("filename=\""); //this is tricky might not necessarily called like this
				//
	if (filename_pos == std::string::npos)
				return "temp.txt";
	size_t filename_end = headers.find("\"", filename_pos + 10);
	return headers.substr(filename_pos + 10, filename_end - filename_pos - 10);
}

statusCode Response::write_file(const std::string &path, const std::string &content, bool append) {
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

void	Response::continue_cgi()
{
	if (_cgi->readCGIfd()) {
		std::cerr << "resume reading error" << std::endl;
		return;
	}
	if (_cgi->isComplete() == true) {
		_body = _cgi->get_result();
		_contentLength = _cgi->get_contentLength();
		buildResponse(static_cast<int>(statusCode::OK), "OK", true); // when cgi double padded?
		_complete = true;
	}
}

void Response::buildResponse(int status, const std::string &message, bool isCGI)
{
	_responseString.append("HTTP/1.1 " + std::to_string(status) + " " + message +
							CRLF);
	if (isCGI) {
		_responseString.append("Content-Length: " + std::to_string(_contentLength) +
							CRLF);
		_responseString.append(_body);
	}
	else {
		if (_body.empty()) {
			return;
		}
		else {
			// std::cout << "Response content length: " << std::to_string(_body.length()) << std::endl;
			_responseString.append("Content-Length: " + std::to_string(_body.length()) +
						  CRLF);
			_responseString.append("Content-Type: " + get_contentType() + CRLF);
			_responseString.append(CRLF + _body);
		}
	}
}

std::string	Response::get_response() { return _responseString; }
std::string	Response::get_contentType() { return _contentType; }
bool		Response::isComplete() { return _complete; }

void Response::printResponse() {
	std::cout << MSG_BORDER << "[Response]" << MSG_BORDER << std::endl;
	std::cout << _responseString << std::endl;
}

// std::unordered_map<std::string, std::string>
// Response::get_args(std::string requestBody, std::string contentType) {
// 	std::unordered_map<std::string, std::string> args;
//
// 	if (contentType == "application/x-www-form-urlencoded")
// 	{
// 		std::string token;
// 		std::istringstream tokenStream(requestBody);
// 		while (std::getline(tokenStream, token, '&'))
// 		{
// 			size_t pos = token.find('=');
// 			if (pos != std::string::npos) {
// 				std::string key = token.substr(0, pos);
// 				std::string value = token.substr(pos + 1);
// 				args[key] = value;
// 			}
// 		}
// 	}
// 	else if (contentType == "multipart/form-data")
// 	{
// 		size_t boundaryPos = contentType.find("boundary=");
// 		if (boundaryPos == std::string::npos)
// 			return args;
// 		std::string boundary = contentType.substr(boundaryPos + 9);
//
// 		std::vector<std::string> parts = split_multipart(requestBody, boundary);
// 		if (parts.empty())
// 			return args;
//
// 		for (const std::string &part : parts)
// 		{
// 			size_t cdPos;
// 			if ((cdPos = part.find("content-disposition: for-data;")) ==
// 				std::string::npos)
// 				return args; // error ?
//
// 			size_t namePos = part.find("name=\"");
// 			if (namePos == std::string::npos)
// 				return args; // error ?
// 			size_t nameEnd = part.find("\"", namePos);
// 			if (nameEnd == std::string::npos)
// 				return args; // error ?
// 			std::string name = part.substr(namePos + 6, nameEnd - namePos - 6);
//
// 			std::string filename;
// 			size_t filenamePos = part.find("filename=\"", cdPos + 31);
// 			if (filenamePos != std::string::npos)
// 			{
// 				size_t filenameEnd = part.find("\"", filenamePos + 10);
// 				if (filenameEnd != std::string::npos)
// 					filename = part.substr(filenamePos + 10,
// 										filenameEnd - filenamePos - 10);
// 			}
// 		}
// 	}
// 	return args;
// }  
