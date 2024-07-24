#include "cgi.hpp"
#include "defines.hpp"
#include "response.hpp"
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

CGI::CGI(const std::shared_ptr<Request> &request, const std::filesystem::path &scriptPath) :
	_request(request), _scriptPath(scriptPath)
{
	parseCGI();
}
 
CGI::~CGI() {}

void CGI::parseCGI()
{
	_cgiArgv.push_back(_scriptPath);
	init_envp();

	if (!_request->get_requestArgs().empty()) {
		for (auto it: _request->get_requestArgs())
			add_to_envp(it.first, it.second, "");
	}
}

void CGI::init_envp()
{
	add_to_envp("GATEWAY_INTERFACE", "CGI/1.1", "");
	add_to_envp("REQUEST_METHOD", _request->get_requestMethod(), "");
	add_to_envp("SCRIPT_FILENAME", _scriptPath, "");
}

// adds variable to envpp if permissed. additive is a specified prefix
bool CGI::add_to_envp(std::string key, std::string value, std::string prefix)
{
	std::string tmp;

	if (validate_key(prefix + key)) {
		tmp = prefix + key;
		for (auto &c : tmp)
			c = toupper(c);
		if (!value.empty())
			tmp += "=" + value;
		std::replace(tmp.begin(), tmp.end(), '-', '_');
		_cgiEnvp.push_back(tmp);
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
std::string CGI::executeCGI()
{
	int pipefd[2];
	int status;
	pid_t pid;
	std::string result;
	
	std::vector<char *> vector_envp;
	for (const auto &str : _cgiEnvp) {
		vector_envp.push_back(const_cast<char *>(str.c_str()));
	}
	vector_envp.push_back(NULL);

	std::vector<char *> vector_argv;
	for (const auto &str : _cgiArgv) {
		vector_argv.push_back(const_cast<char *>(str.c_str()));
	}
	vector_argv.push_back(NULL);

	#ifdef DEBUG
	std::cout << "CGI Argv:" << std::endl;
	for (auto it : _cgiArgv)
		std::cout << it << std::endl;

	std::cout << "CGI envp:" << std::endl;
	for (auto it : _cgiEnvp)
		std::cout << it << std::endl;
	#endif

	char **envp = vector_envp.data();
	char **argv = vector_argv.data();

	if (pipe(pipefd) == -1)
		throw statusCode::INTERNAL_SERVER_ERROR;

	pid = fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		throw statusCode::INTERNAL_SERVER_ERROR;
	}
	else if (pid == 0) {
		// stdin of request body:
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		// LAUNCH
		execve(argv[0], argv, envp);
		std::cerr << "execve failed: " << std::strerror(errno) << std::endl;
		_exit(1);
	}
	else {
		// Parent process
        close(pipefd[1]);

		// write(pipefd_in[1], vars.get_stdin().c_str(), vars.get_stdin().length()); //WRITES TO STDIN
        // Read output from child process
		std::vector<char>buffer(BUFFSIZE);
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], &buffer[0], buffer.size())) > 0)
            result.append(buffer.data(), bytes_read);

		if (bytes_read < 0)
			throw "CGI: read failed";
        close(pipefd[0]);

		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			int exit_status = WEXITSTATUS(status);
			if (exit_status != 0) {
				std::cout << "Child process exited with status: " << exit_status << std::endl;
			}
		}
        return result;
	}
}

	// else {
	// 	close(pipefd_out[1]);
	// 	close(pipefd_in[0]);
	//
	// 	// std::cout << "TO BE WRITTEN IN STDIN OF EXECVE:" << vars.get_stdin() << "__WRITTEN__" << std::endl;
	// 	write(pipefd_in[1], _request->get_body().c_str(), _request->get_body().length()); //WRITES TO STDIN
	// 	close(pipefd_in[1]);
	//
	// 	// // for chunking check if body is empty otherwise keep.
	// 	// std::vector<char> buffer(BUFFSIZE);
	// 	// while ((ssize_t bytes_read = read(pipefd_out[0], &buffer[0], buffer.size())) > 0) {
	// 	// 	std::string part(&buffer[0], bytes_read);
	// 	// 	result.append(part);
	// 	// }
	// 	// if (bytes_read == -1)
	// 	// 	std::cerr << "Read failed" << std::endl;
	// 	// close(pipefd_out[0]);
	// 	//
	// 	waitpid(pid, &status, 0);
	// 	if (WIFEXITED(status)) {
	// 		int exit_status = WEXITSTATUS(status);
	// 		if (exit_status != 0) {
	// 			std::cout << "exit_status: " << exit_status << std::endl;
	// 		}
	// 	}
		// return result;
	// }
