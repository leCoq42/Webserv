#include "cgi.hpp"
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

// REFERENCE :( chapter 4:
// http://www.faqs.org/rfcs/rfc3875.html

//debug
#include <cstdlib>
//debug
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

// std::string cgi::get_header(const std::string &content_type) {
//   std::string header = "Content-Type: " + content_type + "\r\n\r\n";
//   return header;
// }

// std::string cgi::get_start_html(const std::string &title) {
//   std::string html_body =
//       "<html>\n<head>\n<title>" + title + "</title>\n</head>\n<body>\n";
//   return html_body;
// }

// std::string cgi::get_end_html() { return "</body>\n</html>\n"; }

// void cgi::set_header(const std::string &content_type) {
//   _contentType = content_type;
// }

// auto add_to_env = [](const auto& key, const auto& value)
// {
// 	std::string	var;
	
// 	var = key;
// 	var += "=";
// 	var += value;
// 	return (var);
// };

// void	set_env_vars(std::unordered_map<std::string, std::string> headers, char **envpp, std::vector<char *> &envpp_new)
// {
// 	char	**envp_backup = envpp;
// 	int		envp_len = 0;
// 	std::vector<std::string> argv;

// 	//probably unnecesary, and security issue :p
// 	while (*(envpp + envp_len++))
// 		if (*(envpp + envp_len))
// 			argv.push_back(*(envpp + envp_len));
// 	for (const auto& [key, value] : headers)
// 		argv.push_back(add_to_env(key, value));
// 	argv.push_back(add_to_env("fileToUpload", "kut"));
// 	argv.push_back(add_to_env("filename", "hoi.txt"));
// 	for(size_t i = 0; i < argv.size(); ++i)
//         envpp_new.push_back(const_cast<char*>(argv[i].c_str()));
// 	int i = 0;
// 	// envpp_new.push_back(add_to_env"filename=\"hoi.txt\"");
// 	std::cout << "{{{{{{{{{{{{{{{{{{" << "envPP" << "}}}}}}}}}}}}}}}}}}}}}}}}}}}}" << std::endl;
// 	while (envpp_new.data()[i])
// 		std::cout << envpp_new.data()[i++] << std::endl;
// 	std::cout << "{{{{{{{{{{{{{{{{{{" << "envPP" << "}}}}}}}}}}}}}}}}}}}}}}}}}}}}" << std::endl;
// }

void cgi::set_title(const std::string &title) { _title = title; }

//fd in added for request body:
std::string cgi::executeCGI(const std::string &path, const std::string &args, std::shared_ptr<Request> _request, std::string interpreter) {
  int pipefd_out[2];
  int pipefd_in[2];
  int status;
  pid_t pid;
  std::string result;
  int bytes_read;
  std::vector<char *> argv;
//   std::vector<std::string> argv_str;

	if (args.empty())
		;
  std::vector<char *> envpp_new;
  CgiParsing	vars(_request->get_headers(), environ, _request, path, interpreter);
//   std::unordered_map<std::string, std::string> headers, char **environ, ServerStruct &serverinfo, std::shared_ptr<Request> _request, const std::string &path

  // std::cout << "path: " << path << std::endl;

  if (pipe(pipefd_out) == -1 || pipe(pipefd_in) == -1) {
    // throw "Pipe Failed";
    std::cerr << "Pipe Failed" << std::endl;
    exit(EXIT_FAILURE); //never dies?
  }

  pid = fork();
  if (pid < 0) {
    // throw "Fork Failed";
    std::cerr << "Fork Failed" << std::endl;
    exit(EXIT_FAILURE); //never dies?
  } else if (pid == 0) {
	//stdin of request body:
	// set_env_vars(_request->get_headers(), environ, envpp_new);
    close(pipefd_out[0]);
	close(pipefd_in[1]);
    dup2(pipefd_out[1], STDOUT_FILENO);
	dup2(pipefd_in[0], STDIN_FILENO);
    close(pipefd_out[1]);
	close(pipefd_in[0]);

	//add argv variables: might have to be at the envpp
	for (const std::string &arg : vars.get_argv()) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(NULL);
	std::cerr << "LAUNCH\n";
	int i = -1;
	while (argv[++i])
		std::cerr << argv[i] << std::endl;
	std::cerr << "LAUNCH2" << std::endl;

	//add envp variables:
	for (const std::string &arg: vars.get_envp()){
		envpp_new.push_back(const_cast<char *>(arg.c_str()));
	}
	envpp_new.push_back(NULL);
	std::cerr << "ENV\n";
	i = -1;
	while (envpp_new[++i])
		std::cerr << envpp_new[i] << std::endl;
	std::cerr << "ENV2" << std::endl;

	//LAUNCH
    execve(argv.data()[0], argv.data(), envpp_new.data());
	// std::cout << argv[0] << std::endl;
	std::cout << std::strerror(errno) << std::endl;
    std::cout << "Exec failed" << std::endl;
    // throw "Exec failed";
    _exit(EXIT_FAILURE);
  } else {
    close(pipefd_out[1]);
	close(pipefd_in[0]);

	std::cout << "TO BE WRITTEN:" << vars.get_stdin() << "WRITTEN" << std::endl;
	write(pipefd_in[1], vars.get_stdin().c_str(), vars.get_stdin().length());
	close(pipefd_in[1]);
	//for chunking check if body is empty otherwise keep.
    char buffer[BUFF_SIZE];
    while ((bytes_read = read(pipefd_out[0], buffer, sizeof(buffer) - 1)) > 0) {
      std::string part(buffer, bytes_read);
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

// void cgi::createArgs(std::vector<char *> &argv, std::string &path,
//                      std::string &args) {
//   std::vector<std::string> argv_str;
//   argv_str.push_back(path);
//   argv_str.push_back(args);
//   for (const std::string &arg : argv_str) {
//     argv.push_back(const_cast<char *>(arg.c_str()));
//   }
// }

// void cgi::createEnv(std::vector<char *> &envp) {
//   std::vector<std::string> env_str;
// }
