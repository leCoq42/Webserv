#include "Response.hpp"
#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

std::vector<std::string> get_parts(std::string requestBody,
                                   std::string boundary);

Response::Response() : _request(nullptr), _responseString("") {}

Response::Response(std::shared_ptr<Request> request)
    : _request(request), _responseString("") {}

Response::~Response(){};

Response::Response(const Response &src){};

Response &Response::operator=(const Response &rhs) { return *this; };

std::string Response::handleRequest(const Request &request) {
  std::string request_method = request.get_requestMethod();
  try {
    if (request_method == "GET")
      return handleGetRequest(request);
    if (request_method == "POST")
      return handlePostRequest(request);
    if (request_method == "DELETE")
      return handleDeleteRequest(request);
    else
      return buildResponse(static_cast<int>(StatusCode::METHOD_NOT_ALLOWED),
                           "Method Not Allowed", "");
  } catch (const std::exception &e) {
    return buildResponse(static_cast<int>(StatusCode::INTERNAL_SERVER_ERROR),
                         "Internal Server Error", "");
  }
}

std::string Response::handleGetRequest(const Request &request) {
  std::string resourcePath = request.get_uri();

  if (resourcePath.empty())
    resourcePath = "index.html";
  else if (!resourcePath.empty() && resourcePath[0] == '/')
    resourcePath = resourcePath.substr(1);
  else
    return buildResponse(static_cast<int>(400), "Bad Request", "");

  std::string contentType;
  std::size_t extensionPos = resourcePath.find_last_of('.');
  if (extensionPos != std::string::npos) {
    std::string extension = resourcePath.substr(extensionPos + 1);
    if (contentTypes.find(extension) == contentTypes.end())
      return buildResponse(static_cast<int>(StatusCode::UNSUPPORTED_MEDIA_TYPE),
                           "Unsupported Media Type", "");
    contentType = contentTypes.at(extension);
  }

  std::string path = "server/root/" + resourcePath;
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return buildResponse(static_cast<int>(StatusCode::NOT_FOUND), "Not Found",
                         "");

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string body = buffer.str();

  std::unordered_map<std::string, std::string> headers;
  headers["Content-Type"] = contentType;

  return buildResponse(static_cast<int>(StatusCode::OK), "OK", body, headers);
}

std::string Response::handlePostRequest(const Request &request) {
  std::string resourcePath = request.get_uri();
  std::string requestBody = request.get_body();
  std::string requestContentType = request.get_headers().at("Content-Type");

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

  std::string response;
  std::unordered_map<std::string, std::string> bodyArgs =
      get_args(requestBody, requestContentType);

  return buildResponse(static_cast<int>(StatusCode::OK), "OK", response,
                       request.get_headers());
};

std::string Response::handleDeleteRequest(const Request &request) {
  return buildResponse(static_cast<int>(StatusCode::OK), "OK", "");
};

std::unordered_map<std::string, std::string> get_args(std::string requestBody,
                                                      std::string contentType) {
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
    return args;
  }

  std::vector<std::string> get_parts(std::string requestBody,
                                     std::string boundary) {
    size_t pos = 0;
    std::vector<std::string> parts;

    while ((pos = requestBody.find("--" + boundary, pos)) !=
           std::string::npos) {
      size_t start = pos + boundary.length() + 4;
      pos = requestBody.find("--" + boundary, start);
      if (pos != std::string::npos)
        parts.push_back(requestBody.substr(start, pos - start - 2));
    }
    return parts;
  }

  std::string Response::buildResponse(
      int status, const std::string &message, const std::string &body,
      const std::unordered_map<std::string, std::string> &headers) {
    _responseString.append("HTTP/1.1 " + std::to_string(status) + " " +
                           message + "\r\n");

    for (const auto &header : headers) {
      _responseString.append(header.first + ": " + header.second + "/r/n");
    }

    if (!body.empty()) {
      if (headers.count("Content-Length") == 0) {
        _responseString.append(
            "Content-Length: " + std::to_string(body.length()) + "\r\n");
      }
      _responseString.append("\r\n" + body);
    } else {
      _responseString.append("\r\n");
    }

    return _responseString;
  }

  std::string Response::get_response() { return _responseString; }
