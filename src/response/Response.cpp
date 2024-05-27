#include "Response.hpp"
#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Response::Response() : _request(nullptr), _responseString("") {}

Response::Response(std::shared_ptr<Request> request)
    : _request(request), _responseString("") {
  handleRequest(request);
  std::cout << std::endl;
  std::cout << "< Response: >" << std::endl;
  std::cout << _responseString << std::endl;
}

Response::~Response(){};

Response::Response(const Response &src)
    : _request(src._request), _responseString(src._responseString) {}

Response &Response::operator=(const Response &rhs) {
  Response temp(rhs);
  temp.swap(*this);
  return *this;
}

void Response::swap(Response &lhs) {
  std::swap(_request, lhs._request);
  std::swap(_responseString, lhs._responseString);
}

void Response::handleRequest(const std::shared_ptr<Request> &request) {
  std::string request_method = request->get_requestMethod();
  try {
    if (request_method == "GET")
      handleGetRequest(request);
    else if (request_method == "POST")
      handlePostRequest(request);
    else if (request_method == "DELETE")
      handleDeleteRequest(request);
    else
      buildResponse(static_cast<int>(StatusCode::METHOD_NOT_ALLOWED),
                    "Method Not Allowed", "");
  } catch (const std::exception &e) {
    buildResponse(static_cast<int>(StatusCode::INTERNAL_SERVER_ERROR),
                  "Internal Server Error", "");
  }
}

bool Response::handleGetRequest(const std::shared_ptr<Request> &request) {
  std::string resourcePath = request->get_uri();

  if (resourcePath.empty())
    resourcePath = "index.html";
  else if (!resourcePath.empty() && resourcePath[0] == '/')
    resourcePath = resourcePath.substr(1);
  else {
    _responseString = buildResponse(static_cast<int>(400), "Bad Request", "");
    return false;
  }

  std::string contentType;
  std::size_t extensionPos = resourcePath.find_last_of('.');
  if (extensionPos != std::string::npos) {
    std::string extension = resourcePath.substr(extensionPos + 1);
    if (contentTypes.find(extension) == contentTypes.end()) {
      _responseString =
          buildResponse(static_cast<int>(StatusCode::UNSUPPORTED_MEDIA_TYPE),
                        "Unsupported Media Type", "");
      return false;
    }
    contentType = contentTypes.at(extension);
  }

  std::string path = "server/root/" + resourcePath;
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    _responseString =
        buildResponse(static_cast<int>(StatusCode::NOT_FOUND), "Not Found", "");
    return false;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string body = buffer.str();

  std::unordered_map<std::string, std::string> headers;
  headers["Content-Type"] = contentType;

  _responseString = buildResponse(static_cast<int>(StatusCode::OK), "OK", body);

  return true;
}

std::string
Response::handlePostRequest(const std::shared_ptr<Request> &request) {
  std::string resourcePath = request->get_uri();
  std::string requestBody = request->get_body();
  std::string requestContentType = request->get_headers().at("Content-Type");

  if (resourcePath.empty())
    resourcePath = "index.html";
  else if (!resourcePath.empty() && resourcePath[0] == '/')
    resourcePath = resourcePath.substr(1);
  else
    return buildResponse(static_cast<int>(400), "Bad Request", "");

  /* std::istringstream requestStream(requestBody); */
  /* std::string line; */
  /* std::unordered_map<std::string, std::string> formData; */
  /* while (std::getline(requestStream, line)) { */
  /*   std::size_t seperatorPos = line.find('='); */
  /*   if (seperatorPos != std::string::npos) { */
  /*     std::string key = line.substr(0, seperatorPos); */
  /*     std::string value = line.substr(seperatorPos + 1); */
  /*     formData[key] = value; */
  /*   } */
  /* } */

  // std::string response;
  // std::unordered_map<std::string, std::string> bodyArgs =
  //     get_args(requestBody, requestContentType);

  return buildResponse(static_cast<int>(StatusCode::OK), "OK", "Lorem Ipsum");
};

std::string
Response::handleDeleteRequest(const std::shared_ptr<Request> &request) {
  return buildResponse(static_cast<int>(StatusCode::OK), "OK",
                       request->get_body());
};

std::unordered_map<std::string, std::string>
Response::get_args(std::string requestBody, std::string contentType) {
  std::unordered_map<std::string, std::string> args;

  if (contentType == "application/x-www-form-urlencoded") {
    std::string token;
    std::istringstream tokenStream(requestBody);
    while (std::getline(tokenStream, token, '&')) {
      size_t pos = token.find('=');
      if (pos != std::string::npos) {
        std::string key = token.substr(0, pos);
        std::string value = token.substr(pos + 1);
        args[key] = value;
      }
    }
  } else if (contentType == "multipart/form-data") {
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos)
      return args;
    std::string boundary = contentType.substr(boundaryPos + 9);

    std::vector<std::string> parts = get_parts(requestBody, boundary);
    if (parts.empty())
      return args;

    for (const std::string &part : parts) {
      size_t cdPos;
      if ((cdPos = part.find("Content-Disposition: for-data;")) ==
          std::string::npos)
        return args; // error ?

      size_t namePos = part.find("name=\"");
      if (namePos == std::string::npos)
        return args; // error ?
      size_t nameEnd = part.find("\"", namePos);
      if (nameEnd == std::string::npos)
        return args; // error ?
      std::string name = part.substr(namePos + 6, nameEnd - namePos - 6);

      std::string filename;
      size_t filenamePos = part.find("filename=\"", cdPos + 31);
      if (filenamePos != std::string::npos) {
        size_t filenameEnd = part.find("\"", filenamePos + 10);
        if (filenameEnd != std::string::npos)
          filename =
              part.substr(filenamePos + 10, filenameEnd - filenamePos - 10);
      }
    }
  }
  return args;
}

std::vector<std::string> Response::get_parts(std::string requestBody,
                                             std::string boundary) {
  size_t pos = 0;
  std::vector<std::string> parts;

  while ((pos = requestBody.find("--" + boundary, pos)) != std::string::npos) {
    size_t start = pos + boundary.length() + 4;
    pos = requestBody.find("--" + boundary, start);
    if (pos != std::string::npos)
      parts.push_back(requestBody.substr(start, pos - start - 2));
  }
  return parts;
}

std::string Response::buildResponse(int status, const std::string &message,
                                    const std::string &body) {
  _responseString.append("HTTP/1.1 " + std::to_string(status) + " " + message +
                         "\r\n");

  _responseString.append("Content-Type: text/html\r\n");

  if (!body.empty()) {
    _responseString.append("Content-Length: " + std::to_string(body.length()) +
                           "\r\n");
    _responseString.append("\r\n" + body);
  } else {
    _responseString.append("\r\n");
  }
  return _responseString;
}

std::string Response::get_response() { return _responseString; }

void Response::printResponse() {
  std::cout << "< Response: >" << std::endl;
  std::cout << _responseString;
}
