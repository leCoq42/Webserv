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
  std::string header = "Content-Type: " + content_type + "\r\n\r\n";
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

std::string cgi::executeCGI(const std::string &path, const std::string &args) {
  int pipefd[2];
  int status;
  pid_t pid;
  std::string result;
  int bytes_read;
  std::vector<char *> argv;
  std::vector<std::string> argv_str;

  // std::cout << "path: " << path << std::endl;

  if (pipe(pipefd) == -1) {
    // throw "Pipe Failed";
    std::cerr << "Pipe Failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  pid = fork();
  if (pid < 0) {
    // throw "Fork Failed";
    std::cerr << "Fork Failed" << std::endl;
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    argv_str.push_back(path);
    argv_str.push_back(args);
    for (const std::string &arg : argv_str) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(NULL);

    execve(argv[0], argv.data(), environ);
    std::cout << "Exec failed" << std::endl;
    // throw "Exec failed";
    _exit(EXIT_FAILURE);
  } else {
    close(pipefd[1]);

    char buffer[BUFF_SIZE];
    while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
      std::string part(buffer, bytes_read);
      result.append(part);
    }
    if (bytes_read == -1) {
      std::cerr << "Read failed" << std::endl;
      // throw "Read failed";
    }
    close(pipefd[0]);

    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      int exit_status = WEXITSTATUS(status);
      if (exit_status != 0) {
        std::cout << "exit_status: " << exit_status << std::endl;
      }
    }
    return result;
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

// void cgi::createEnv(std::vector<char *> &envp) {
//   std::vector<std::string> env_str;
// }
