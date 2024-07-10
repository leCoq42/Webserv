#pragma once

#include "request.hpp"
#include <memory>

class cgi {
public:
  cgi();
  cgi(const std::string &contentType);
  cgi(const std::string &contentType, const std::string &title);
  ~cgi();

  std::string get_header(const std::string &content_type);
  std::string get_start_html(const std::string &title);
  std::string get_end_html();

  void set_header(const std::string &content_type);
  void set_title(const std::string &title);

  std::string get_contentType() const { return _contentType; }
  std::string get_title() const { return _title; }

  void createArgs(std::vector<char *> &argv, std::string &path,
                  std::string &args);
  // void createEnv(std::vector<char *> &envp);

  std::string executeCGI(const std::string &path, const std::string &args,
                         std::shared_ptr<Request> _request,
                         std::string interpreter);

private:
  std::string _contentType;
  std::string _title;
};
