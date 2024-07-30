#include "cgi.hpp"
#include "defines.hpp"
#include "log.hpp"
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
#include <unistd.h>

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
	_cgiArgv.push_back(nullptr);

	init_envp();
	if (!_request->get_requestArgs().empty()) {
		for (auto it: _request->get_requestArgs())
			add_to_envp(it.first, it.second, "HTTP_");
	}
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

void CGI::executeScript()
{
    int 	pipeIn[2];
	int		pipeOut[2];
	pid_t	pid;

	// if (pipe(pipefd) == -1)
    if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
		_log.logError("Failed to create pipe");

    pid = fork();
    if (pid == -1) {
        close(pipeIn[READ]);
        close(pipeIn[WRITE]);
		close(pipeOut[READ]);
		close(pipeOut[WRITE]);
        _log.logError("Fork Failed.");
		return;
    }

	if (pid == 0) {  // Child process
		close(pipeIn[WRITE]);
		close(pipeOut[READ]);

		dup2(pipeIn[READ], STDIN_FILENO);
		close(pipeIn[READ]);

        dup2(pipeOut[WRITE], STDOUT_FILENO);
		close(pipeOut[WRITE]);

        // Execute the script
        execve(_cgiArgv.data()[0], _cgiArgv.data(), _cgiEnvp.data());
		_log.logError("Execve error.");
		_exit(1);
    }
	else {  // Parent process
    	// close(pipeOut[READ]);  // Close write end of pipe
		close(pipeIn[READ]);
		close(pipeIn[WRITE]);  // Close write end of input pipe
        close(pipeOut[WRITE]);  // Close write end of output pipe

		std::vector<char> buffer(BUFFSIZE);
        ssize_t bytesRead;

        while ((bytesRead = read(pipeOut[READ], &buffer[0], buffer.size())) > 0)
            _result.append(buffer.data(), bytesRead);

		close(pipeOut[READ]);
		if (bytesRead < 0 && (errno != EAGAIN || errno != EWOULDBLOCK))
			_log.logError("CGI: read failed");

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			_log.logError("Script exited with non-zero status");
    }
}

ssize_t	CGI::execute_script(int cgi_fd)
{
	(void)cgi_fd;

	return 0;
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
