#include "cgi.hpp"
#include "defines.hpp"
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <cerrno>
#include <clocale>
#include <cmath>
#include <cstring>
#include <iostream>
#include <algorithm>

// REFERENCE :( chapter 4:
// http://www.faqs.org/rfcs/rfc3875.html

CGI::CGI() : _request(nullptr) {}

CGI::CGI(const std::shared_ptr<Request> &request, const std::string &interpreter) : _request(request), _interpreter(interpreter) {
	parseCGI();
}
 
CGI::~CGI() {}

void CGI::parseCGI() {
	init_envp();
	_cgiArgv.push_back(const_cast<char *>(_request->get_requestPath().c_str()));
	for (auto it: _request->get_requestArgs())
		add_to_envp(it.first, it.second, "");
}

void CGI::init_envp() {
	add_to_envp("GATEWAY_INTERFACE", "CGI/1.1", "");
	add_to_envp("REQUEST_METHOD", _request->get_requestMethod(), "");
	add_to_envp("SCRIPT_FILENAME", _request->get_requestPath(), "");
}

// adds variable to envpp if permissed. additive is a specified prefix
bool CGI::add_to_envp(std::string key, std::string value, std::string prefix) {
	std::string tmp;

	if (validate_key(prefix + key)) {
		tmp = prefix + key;
		for (auto &c : tmp)
			c = toupper(c);
		if (!value.empty())
			tmp += "=" + value;
		std::replace(tmp.begin(), tmp.end(), '-', '_');
		_cgiEnvp.push_back(const_cast<char *>(tmp.c_str()));
		return true;
	}
	return false;
}

bool CGI::validate_key(std::string key) {
	for (auto &c : key)
	c = toupper(c);

	auto it = std::find(metaVarNames.begin(), metaVarNames.end(), key);
	if (it != metaVarNames.end())
		return true;

	it = std::find(custom_var_prefixes.begin(), custom_var_prefixes.end(), key);
	if (it != custom_var_prefixes.end())
		return true;

	return false;
}

// fd in added for request body:
std::string CGI::executeCGI() {
	int pipefd_out[2];
	int pipefd_in[2];
	int status;
	pid_t pid;
	std::string result;

	if (pipe(pipefd_out) == -1 || pipe(pipefd_in) == -1) {
		std::cerr << "Pipe Failed" << std::endl;
		return nullptr;
	}

	std::cout << "CGI file path!!: " << _cgiArgv.data()[0] << std::endl;
	pid = fork();
	if (pid < 0) {
		std::cerr << "Fork Failed" << std::endl;
		return nullptr;
	}
	else if (pid == 0) {
		// stdin of request body:
		close(pipefd_out[0]);
		close(pipefd_in[1]);
		dup2(pipefd_out[1], STDOUT_FILENO);
		dup2(pipefd_in[0], STDIN_FILENO);
		close(pipefd_out[1]);
		close(pipefd_in[0]);

		std::cout << ">>> CGI PATH = " << _cgiArgv[0] << std::endl;

		// LAUNCH
		execve(_cgiArgv[0], _cgiArgv.data(), _cgiEnvp.data());
		std::cerr << std::strerror(errno) << std::endl;
		std::cerr << "execve failed" << std::endl;
		return "";
	}
	else {
		close(pipefd_out[1]);
		close(pipefd_in[0]);

		// std::cout << "TO BE WRITTEN IN STDIN OF EXECVE:" << vars.get_stdin() << "__WRITTEN__" << std::endl;
		write(pipefd_in[1], _request->get_body().c_str(), _request->get_body().length()); //WRITES TO STDIN
		close(pipefd_in[1]);

		// // for chunking check if body is empty otherwise keep.
		// std::vector<char> buffer(BUFFSIZE);
		// while ((ssize_t bytes_read = read(pipefd_out[0], &buffer[0], buffer.size())) > 0) {
		// 	std::string part(&buffer[0], bytes_read);
		// 	result.append(part);
		// }
		// if (bytes_read == -1)
		// 	std::cerr << "Read failed" << std::endl;
		// close(pipefd_out[0]);
		//
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
