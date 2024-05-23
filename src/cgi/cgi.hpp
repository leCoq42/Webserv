#pragma once

#include <iostream>
#include <vector>

class cgi {
public:
  cgi();
  cgi(const std::string &contentType, const std::string &title);
  cgi(const std::string &title);
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
  void createEnv(std::vector<char *> &envp);

  void executeCGI(const std::string &path, const std::string &args);

private:
  std::string _contentType;
  std::string _title;
};
