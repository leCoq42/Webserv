#include "cgi.hpp"
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

const std::string CGI_DIR = "/var/www/cgi-bin";
constexpr size_t BUFF_SIZE = 1024;

cgi::cgi() : _contentType(""), _title("") {}

cgi::cgi(const std::string &contentType, const std::string &title)
    : _contentType(contentType), _title(title) {}

cgi::cgi(const std::string &title) : _title(title) {}

cgi::~cgi() {}

std::string cgi::get_header(const std::string &content_type) {
  std::string header = "Content-type: " + content_type + "\r\n\r\n";
  return header;
}

std::string cgi::get_start_html(const std::string &title) {
  std::string html_body =
      "<html>\n<head>\n<title>" + title + "</title>\n</head>\n<body>\n";
  return html_body;
}

std::string cgi::get_end_html() { return "</body>\n</html>\n"; }

void cgi::set_header(const std::string &content_type) {
  _contentType = content_type;
}

void cgi::set_title(const std::string &title) { _title = title; }

void cgi::executeCGI(const std::string &path, const std::string &args) {
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    throw "Pipe Failed";
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();
  if (pid < 0) {
    throw "Fork Failed";
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    std::vector<char *> argv;
    std::vector<std::string> argv_str;
    argv_str.push_back(path);
    argv_str.push_back(args);
    for (const std::string &arg : argv_str) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }

    execve(argv[0], argv.data(), environ);
    throw "Exec failed";
    _exit(EXIT_FAILURE);
  } else {
    close(pipefd[1]);

    char buffer[BUFF_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
      buffer[bytes_read] = '\0';
      std::cout << buffer;
    }
    if (bytes_read == -1) {
      throw "Read failed";
    }
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      return;
    } else {
      throw "Child process did not terminate normally";
    }
  }
}

void cgi::createArgs(std::vector<char *> &argv, std::string &path,
                     std::string &args) {
  std::vector<std::string> argv_str;
  argv_str.push_back(path);
  argv_str.push_back(args);
  for (const std::string &arg : argv_str) {
    argv.push_back(const_cast<char *>(arg.c_str()));
  }
}

void cgi::createEnv(std::vector<char *> &envp) {
  std::vector<std::string> env_str;
}
