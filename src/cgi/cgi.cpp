#include "cgi.hpp"
#include "defines.hpp"
#include "log.hpp"
#include <cstddef>
#include <cstring>
#include <sys/socket.h>
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

CGI::CGI(const std::shared_ptr<Request> &request, const std::filesystem::path &scriptPath, const std::string &interpreter) :
	_request(request), _scriptPath(scriptPath), _interpreter(interpreter), _result(""),
	_contentLength(0), _cgiFD(0), _complete(false)
{
	parseCGI();
	executeScript();
	if (_complete)
		calculateContentLength();
}
 
CGI::~CGI() {}

void CGI::parseCGI()
{
	if (!_interpreter.empty())
		_cgiArgv.push_back(const_cast<char *>(_interpreter.c_str()));
	_cgiArgv.push_back(const_cast<char *>(_scriptPath.c_str()));
	_cgiArgv.push_back(NULL);

	init_envp();
	if (!_request->get_requestArgs().empty()) {
		for (auto it: _request->get_requestArgs())
			add_to_envp(it.first, it.second, "HTTP_");
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

void CGI::executeScript()
{
    int 	pipeServertoCGI[2];
	int		pipeCGItoServer[2];
	pid_t	pid;

	std::vector<char*> envp;
	for (const auto& var : _cgiEnvp) {
		envp.push_back(const_cast<char*>(var.c_str()));
	}

envp.push_back(nullptr);

    if (pipe(pipeServertoCGI) == -1 || pipe(pipeCGItoServer) == -1)
		_log.logError("Failed to create pipe");

    pid = fork();
    if (pid == -1) {
        close(pipeServertoCGI[READ]);
        close(pipeServertoCGI[WRITE]);
		close(pipeCGItoServer[READ]);
		close(pipeCGItoServer[WRITE]);
        _log.logError("Fork Failed.");
		return;
    }

	if (pid == 0) {  // Child process
		close(pipeServertoCGI[WRITE]);
		close(pipeCGItoServer[READ]);

		dup2(pipeServertoCGI[READ], STDIN_FILENO);
		close(pipeServertoCGI[READ]);

        dup2(pipeCGItoServer[WRITE], STDOUT_FILENO);
		close(pipeCGItoServer[WRITE]);

        // Execute the script
        execve(_cgiArgv.data()[0], _cgiArgv.data(), envp.data());
		_log.logError("Execve error.");
		_exit(1);
    }
	else {  // Parent process
    	// close(pipeOut[READ]);  // Close write end of pipe
		close(pipeServertoCGI[READ]);
		close(pipeServertoCGI[WRITE]);  // Close write end of input pipe
        close(pipeCGItoServer[WRITE]);  // Close write end of output pipe

		_cgiFD = pipeCGItoServer[READ];
		readCGIfd();

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			_log.logError("Script exited with non-zero status");
    }
}

int	CGI::readCGIfd()
{
	ssize_t bytesRead;
	std::vector<char> buffer(BUFFSIZE);

	bytesRead = read(_cgiFD, &buffer[0], BUFFSIZE);
	if (bytesRead == 0) {
		close(_cgiFD);
		_complete = true;
		calculateContentLength();
		return 0;
	}
	else if (bytesRead > 0) {
		_complete = false;
		_result.append(buffer.data(), bytesRead);
		return 0;
	}
	else {
		_complete = true;
		return 1;
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

size_t		CGI::get_contentLength() { return _contentLength; }
std::string	CGI::get_result() { return _result; }
int			CGI::get_cgiFD() { return _cgiFD; }
bool		CGI::isComplete() { return _complete; }
