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

Response::Response(ServerStruct &config) : _request(nullptr), _responseString(""), _config(config), _fileAccess(config) {}

Response::Response(std::shared_ptr<Request> request, ServerStruct &config)
    : _request(request), _responseString(""), _contentType(""), _bufferFile(""), _config(config), _fileAccess(config)
{
	handleRequest(request);
	printResponse();
}

Response::Response(std::shared_ptr<Request> request, ServerStruct &config, std::string filename)
    : _request(request), _responseString(""), _contentType(""), _bufferFile(filename), _config(config), _fileAccess(config)
{
	handleRequest(request);
	printResponse();
}

Response::~Response() {}

Response::Response(const Response &src)
    : _request(src._request), _responseString(src._responseString),
      _contentType(src._contentType), _config(src._config), _fileAccess(src._config) {}

Response &Response::operator=(const Response &rhs) {
	Response temp(rhs);
	temp.swap(*this);
	return *this;
}

void Response::swap(Response &lhs) {
	std::swap(_request, lhs._request);
	std::swap(_responseString, lhs._responseString);
	std::swap(_contentType, lhs._contentType);
}

void Response::handleRequest(const std::shared_ptr<Request> &request)
{
	int	return_code = 0;
	std::string request_method = request->get_requestMethod();

	try {
		_requestPath = _fileAccess.isFilePermissioned(request->get_uri(), return_code);
		std::cout << "PATH:" << _requestPath << std::endl;
		if (return_code)
		{
			_requestPath = _fileAccess.getErrorPage(return_code); // wrong place
			buildResponse(static_cast<int>(return_code), "Not Found", "");
		}
		else if (request_method == "GET" && _fileAccess.allowedMethod("GET"))
			handleGetRequest(request);
		else if (request_method == "POST" && _fileAccess.allowedMethod("POST"))
			handlePostRequest(request);
		else if (request_method == "DELETE" && _fileAccess.allowedMethod("DELETE"))
			handleDeleteRequest(request);
		else
			buildResponse(static_cast<int>(statusCode::METHOD_NOT_ALLOWED),
						"Method Not Allowed", "");
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		buildResponse(static_cast<int>(statusCode::INTERNAL_SERVER_ERROR),
					"Internal Server Error", "");
	}
}

bool Response::handleGetRequest(const std::shared_ptr<Request> &request) {
	std::string body;
	std::stringstream buffer;
	bool isCGI = false;

	if (!_requestPath.empty() && _requestPath.has_extension())
	{
		std::unordered_map<std::string, std::string>::const_iterator it =
			contentTypes.find(_requestPath.extension());
		if (it == contentTypes.end())
		{
			_responseString =
				buildResponse(static_cast<int>(statusCode::UNSUPPORTED_MEDIA_TYPE),
				  "Unsupported Media Type", "");
			return false;
		}
		_contentType = it->second;

		if (interpreters.find(_requestPath.extension()) == interpreters.end())
		{
			body = readFileToBody(_requestPath);
			if (body.empty())
				return false;
		}
		else
		{
			isCGI = true;
			cgi CGI(_contentType);
			body = CGI.executeCGI(_requestPath, "", _request,
									interpreters.at(_requestPath.extension()));
		}
	}
	else
		body = list_dir(_requestPath, request->get_uri(), request->get_referer());
	_responseString = buildResponse(static_cast<int>(statusCode::OK), "OK", body,
			 						isCGI); // when cgi double padded?
	return true;
}

bool Response::handlePostRequest(const std::shared_ptr<Request> &request) {
	std::string requestBody = request->get_body();
	std::string requestContentType = request->get_contentType();
	std::filesystem::path path = "html/";
	std::filesystem::path resourcePath = request->get_uri();
	std::string body;
	bool isCGI = false;

	if (!resourcePath.empty() && resourcePath.string()[0] == '/')
		resourcePath = resourcePath.string().substr(1);
	if (resourcePath.empty())
		resourcePath = "index.html";

	std::cout << "ResourcePath:" << resourcePath << std::endl;
	if (resourcePath.has_extension())
	{
		if (interpreters.find(resourcePath.extension()) != interpreters.end())
		{
			isCGI = true;
			path.append(resourcePath.string());
			cgi CGI(_contentType);
			body = CGI.executeCGI(path, "", _request,
									interpreters.at(resourcePath.extension()));
		}
		else
		{
			buildResponse(static_cast<int>(statusCode::NO_CONTENT), "No Content", "");
			return true;
		}
	}
	if (requestContentType == "multipart/form-data")
	{
		handle_multipart();
		return true;
	}
	buildResponse(static_cast<int>(statusCode::OK), "OK", body, isCGI);
	return true;
}

bool Response::handleDeleteRequest(const std::shared_ptr<Request> &request)
{
	std::filesystem::path Path = request->get_uri();

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

void Response::handle_multipart() {
	statusCode status = statusCode::OK;
	std::string requestBody = _request->get_body();
	std::string boundary = _request->get_boundary();
	std::string filename = "";
	std::string responseBody = "";
	bool		append = false;

	std::cout << MSG_BORDER << "[MULTIPART REQUEST]" << MSG_BORDER << std::endl;
	if (_bufferFile.empty())
	{
		std::vector<std::string> parts = split_multipart(requestBody, boundary);
		for (const std::string &part: parts)
		{
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
				std::cout << "Part without content" << std::endl;
				continue;
			}

			filename = extract_filename(headers);

			std::cout << MSG_BORDER << "[part content:]" << MSG_BORDER << "\n" << content << std::endl;

			status = write_file(      "html/uploads/" + filename, content, append);
			if (status != statusCode::OK)
				break;
			append = true;
			// _request->set_bufferFile(_requestPath.root_path().append("html/uploads/" + filename));
			// _request->set_startContentLength(requestBody.length());//content.length());
		}
		if (status == statusCode::OK)
			responseBody = readFileToBody("html/upload_success.html");
		else
			responseBody = readFileToBody("html/standard_404.html");
	}
	else {//TODO: should be able to run cgi as well
		status = statusCode::OK;
	}
	buildResponse(static_cast<int>(status), statusCodeMap.at(status), responseBody);
}

std::vector<std::string> Response::split_multipart(std::string requestBody,
                                             std::string boundary)
{
	std::vector<std::string> parts;
	std::string fullBoundary = "--" + boundary;
	size_t pos = 0;

	while (pos < requestBody.size())
	{
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

std::string Response::extract_filename(const std::string &headers)
{
	size_t filename_pos = headers.find("filename=\""); //this is tricky might not necessarily called like this
				//
	if (filename_pos == std::string::npos)
				return "temp.txt";
	size_t filename_end = headers.find("\"", filename_pos + 10);
	return headers.substr(filename_pos + 10, filename_end - filename_pos - 10);
}

statusCode Response::write_file(const std::string &path, const std::string &content, bool append)
{
	std::ofstream file;
	if (append)
		file.open(path, std::ios::binary | std::ios::app);
	else
		file.open(path, std::ios::binary);
	if (file.is_open())
	{
		file.write(content.c_str(), content.length());
		file.close();
		return statusCode::OK;
	}
	else
		return statusCode::INTERNAL_SERVER_ERROR;
}

std::string Response::buildResponse(int status, const std::string &message,
                                    const std::string &body, bool isCGI)
{
	_responseString.append("HTTP/1.1 " + std::to_string(status) + " " + message +
							CRLF);
	if (!body.empty() && !isCGI)
	{
		_responseString.append("Content-Length: " + std::to_string(body.length()) +
							CRLF);
	}
	if (_request->get_keepAlive())
	{
		_responseString.append(
			"Keep-Alive: timeout=" + std::to_string(KEEP_ALIVE_TIMOUT) +
			", max=" + std::to_string(KEEP_ALIVE_N) + CRLF);
	}
	if (isCGI)
	{
		_responseString.append(body);
	}
	else
	{
		_responseString.append("Content-Type: " + get_contentType() + CRLF);
		_responseString.append(CRLF + body);
	}
	return _responseString;
}

std::string Response::get_response() { return _responseString; }
std::string Response::get_contentType() { return _contentType; }

void Response::printResponse()
{
	// std::cout << MSG_BORDER << "[Response]" << MSG_BORDER << std::endl;
	// std::cout << _responseString << std::endl;
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
