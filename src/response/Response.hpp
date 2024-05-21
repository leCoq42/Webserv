#pragma once

#include "../request/Request.hpp"
#include <memory>
#include <unordered_map>

enum class StatusCode {
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
  Response();
  Response(std::shared_ptr<Request> request);
  ~Response();

  Response(const Response &src);
  Response &operator=(const Response &rhs);
  void swap(Response &tmp);

  std::string handleRequest(const Request &request);
  std::string get_response();

private:
  std::shared_ptr<Request> _request;
  std::string _responseString;

  std::string handleGetRequest(const Request &request);
  std::string handlePostRequest(const Request &request);
  std::string handleDeleteRequest(const Request &request);
  std::string get_args(std::string requestBody, std::string contentType);

  std::unordered_map<std::string, std::string>

  std::string buildResponse(
      int status, const std::string &message, const std::string &body,
      const std::unordered_map<std::string, std::string> &headers = {});

  static const inline std::unordered_map<std::string, std::string> contentTypes{
      {"html", "text/html"},
      {"txt", "text/plain"},

      {"jpeg", "image/jpeg"},
      {"jpg", "image/jpg"},
      {"png", "image/png"},
      {"gif", "image/gif"},

      {"x-www-form-urlencoded", "application/x-www-form-urlencoded"},
      {"form-data", "multipart/form-data"}};
};
