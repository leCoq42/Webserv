#include "cgi.hpp"
#include "defines.hpp"
#include <cstddef>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <cerrno>
#include <clocale>
#include <cmath>
#include <cstring>
#include <algorithm>

// REFERENCE :( chapter 4:
// http://www.faqs.org/rfcs/rfc3875.html

CGI::CGI() : _request(nullptr) {}

CGI::CGI(const std::shared_ptr<Request> &request, const std::filesystem::path &scriptPath, const std::string &interpreter) :
	_request(request), _scriptPath(scriptPath), _interpreter(interpreter)
{
	parseCGI();
	executeScript();
	calculateContentLength();
}
 
CGI::~CGI() {}

void CGI::parseCGI()
{
	if (!_interpreter.empty())
		_cgiArgv.push_back(const_cast<char *>(_interpreter.c_str()));
	_cgiArgv.push_back(const_cast<char *>(_scriptPath.c_str()));
	init_envp();

	if (!_request->get_requestArgs().empty()) {
		for (auto it: _request->get_requestArgs())
			add_to_envp(it.first, it.second, "");
	}
	_cgiArgv.push_back(nullptr);
	_cgiEnvp.push_back(nullptr);
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

void CGI::executeScript() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        throw std::runtime_error("Failed to create pipe");
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        throw std::runtime_error("Failed to fork");
    }

    if (pid == 0) {  // Child process
        close(pipefd[0]);  // Close read end of pipe
        dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to pipe
        dup2(pipefd[1], STDERR_FILENO);  // Redirect stderr to pipe
        close(pipefd[1]);

        // Execute the script
        execve(_cgiArgv.data()[0], _cgiArgv.data(), _cgiEnvp.data());

        // If execve fails, exit
        _exit(1);
    }
	else {  // Parent process
    	close(pipefd[1]);  // Close write end of pipe

        std::string output;
		std::vector<char> buffer(BUFFSIZE);
        ssize_t bytesRead;

        while ((bytesRead = read(pipefd[0], &buffer[0], buffer.size())) > 0) {
            output.append(buffer.data(), bytesRead);
        }
        close(pipefd[0]);
		if (bytesRead < 0)
			throw "CGI: read failed";

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            throw std::runtime_error("Script exited with non-zero status");
        }
        _result = output;
    }
}

void	CGI::calculateContentLength()
{
	std::string body;

	size_t start = _result.find(CRLFCRLF);
	if (start != std::string::npos) {
		body = _result.substr(start + 4);
		_contentLength = body.length();
	}
	else
		_contentLength = 0;

}

size_t	CGI::get_contentLength() { return _contentLength; }
std::string CGI::get_result() {return _result; }

// std::string	CGI::get_contentType()
// {
// 	std::string contentType;
//
// 	auto start = _result.find("Content-Type");
// 	if (start != std::string::npos) {
// 		auto end = _result.find(CRLF, start);
// 		if (end != std::string::npos)
// 			contentType = _result.substr(start, end);
// 		else
// 			contentType = _result.substr(start);
// 	}
// 	return contentType;
// }

// // fd in added for request body:
// std::string CGI::executeScript()
// {
// 	int pipefd[2];
// 	int status;
// 	pid_t pid;
// 	std::string result;
//
// 	#ifdef DEBUG
// 	std::cout << "CGI Argv:" << std::endl;
// 	for (auto it : _cgiArgv)
// 		std::cout << it << std::endl;
//
// 	std::cout << "CGI envp:" << std::endl;
// 	for (auto it : _cgiEnvp)
// 		std::cout << it << std::endl;
// 	#endif
//
// 	if (pipe(pipefd) == -1)
// 		throw statusCode::INTERNAL_SERVER_ERROR;
//
// 	pid = fork();
// 	if (pid < 0) {
// 		close(pipefd[0]);
// 		close(pipefd[1]);
// 		throw statusCode::INTERNAL_SERVER_ERROR;
// 	}
// 	else if (pid == 0) {
// 		// stdin of request body:
// 		close(pipefd[0]);
// 		dup2(pipefd[1], STDOUT_FILENO);
// 		close(pipefd[1]);
//
// 		// LAUNCH
// 		execve(_scriptPath.c_str(), _cgiArgv.data(), _cgiEnvp.data());
// 		// throw ("execve failed: " + (std::string)std::strerror(errno));
// 		std::cerr << "execve failed: " << std::strerror(errno) << std::endl;
// 		_exit(1);
// 	}
// 	else {
// 		// Parent process
//         close(pipefd[1]);
//
// 		// write(pipefd_in[1], vars.get_stdin().c_str(), vars.get_stdin().length()); //WRITES TO STDIN
//         // Read output from child process
// 		std::vector<char>buffer(BUFFSIZE);
//         ssize_t bytes_read;
//         while ((bytes_read = read(pipefd[0], &buffer[0], buffer.size())) > 0)
//             result.append(buffer.data(), bytes_read);
//
// 		if (bytes_read < 0)
// 			throw "CGI: read failed";
//         close(pipefd[0]);
//
// 		waitpid(pid, &status, 0);
// 		if (WIFEXITED(status)) {
// 			int exit_status = WEXITSTATUS(status);
// 			if (exit_status != 0) {
// 				std::cout << "Child process exited with status: " << exit_status << std::endl;
// 			}
// 		}
//         return result;
// 	}
// }

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
