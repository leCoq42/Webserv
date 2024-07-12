#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <unistd.h>

// REFERENCE :( chapter 4:
// http://www.faqs.org/rfcs/rfc3875.html

#include "cgi.hpp"
#include "cgiParsing.hpp"
#include <cerrno>
#include <clocale>
#include <cmath>
#include <cstring>
#include <iostream>

const std::string CGI_DIR = "/var/www/cgi-bin";
constexpr size_t BUFF_SIZE = 1024;

cgi::cgi() : _contentType(""), _title("") {}

cgi::cgi(const std::string &contentType)
    : _contentType(contentType), _title("") {}

cgi::cgi(const std::string &contentType, const std::string &title)
    : _contentType(contentType), _title(title) {}

cgi::~cgi() {}

void cgi::set_title(const std::string &title) { _title = title; }

// fd in added for request body:
std::string cgi::executeCGI(const std::string &path, const std::string &args,
                            std::shared_ptr<Request> _request,
                            std::string interpreter) {
  int pipefd_out[2];
  int pipefd_in[2];
  int status;
  pid_t pid;
  std::string result;
  int bytes_read;
  std::vector<char *> argv;

  if (args.empty())
    ;
  std::vector<char *> envpp_new;
    extern char **environ;
  CgiParsing vars(_request->get_headers(), environ, _request, path,
                  interpreter); // parses all the magic

  if (pipe(pipefd_out) == -1 || pipe(pipefd_in) == -1) {
    // throw "Pipe Failed";
    std::cerr << "Pipe Failed" << std::endl;
    exit(EXIT_FAILURE); // never dies?
  }

  pid = fork();
  if (pid < 0) {
    // throw "Fork Failed";
    std::cerr << "Fork Failed" << std::endl;
    exit(EXIT_FAILURE); // never dies?
  } else if (pid == 0) {
    // stdin of request body:
    close(pipefd_out[0]);
    close(pipefd_in[1]);
    dup2(pipefd_out[1], STDOUT_FILENO);
    dup2(pipefd_in[0], STDIN_FILENO);
    close(pipefd_out[1]);
    close(pipefd_in[0]);

    // add argv variables: might have to be at the envpp, from std::string vector in CgiParsing object to char * vector
    for (const std::string &arg : vars.get_argv()) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(NULL);
    // std::cerr << "DEBUG SHOW ARGVS PASSED:\n";
    // int i = -1;
    // while (argv[++i])
    // 	std::cerr << argv[i] << std::endl;
    // std::cerr << "LAUNCH2" << std::endl;

    // add envp variables: , from std::string vector in CgiParsing object to char * vector
    for (const std::string &arg : vars.get_envp()) {
      envpp_new.push_back(const_cast<char *>(arg.c_str()));
    }
    envpp_new.push_back(NULL);
    // std::cerr << "DEBUG SHOW ENV PASSED:\n";
    // i = -1;
    // while (envpp_new[++i])
    // 	std::cerr << envpp_new[i] << std::endl;
    // std::cerr << "ENV2" << std::endl;

    // LAUNCH
    execve(argv.data()[0], argv.data(), envpp_new.data());
    // std::cout << argv[0] << std::endl;
    std::cout << std::strerror(errno) << std::endl;
    std::cout << "Exec failed" << std::endl;
    // throw "Exec failed";
    _exit(EXIT_FAILURE);
  } else {
    close(pipefd_out[1]);
    close(pipefd_in[0]);

    // std::cout << "TO BE WRITTEN IN STDIN OF EXECVE:" << vars.get_stdin() << "__WRITTEN__" << std::endl;
    write(pipefd_in[1], vars.get_stdin().c_str(), vars.get_stdin().length()); //WRITES TO STDIN
    close(pipefd_in[1]);
    // for chunking check if body is empty otherwise keep.
	std::vector<char> buffer(BUFF_SIZE);
    while ((bytes_read = read(pipefd_out[0], &buffer[0], buffer.size())) > 0) {
      std::string part(&buffer[0], bytes_read);
      result.append(part);
    }
    if (bytes_read == -1) {
      // std::cerr << "Read failed" << std::endl;
      throw "Read failed";
    }
    close(pipefd_out[0]);

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
