#include "cgi.hpp"
#include "defines.hpp"
#include "log.hpp"
#include <cstdlib>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
#include <cerrno>
#include <cstring>
#include <algorithm>

CGI::CGI() :
	_request(nullptr), _scriptPath(""), _interpreter(""), _result(""),
	_contentLength(0), _cgiFD(0), _complete(false)
{}


CGI::CGI(const std::shared_ptr<Request> &request, const std::filesystem::path &scriptPath, const std::string &interpreter, std::shared_ptr<Log> log) :
	_log(log), _request(request), _scriptPath(scriptPath), _interpreter(interpreter), _result(""),
	_contentLength(0), _cgiFD(0), _complete(false), _pid(0)
{
	parseCGI();
	executeScript();
	if (_complete)
		calculateContentLength();
}


CGI::CGI(const CGI &src) :
	_log(src._log), _request(src._request), _scriptPath(src._scriptPath), _interpreter(src._interpreter),
	_cgiArgv(src._cgiArgv), _cgiEnvp(src._cgiEnvp), _result(src._result),
	_contentLength(src._contentLength), _cgiFD(src._cgiFD), _complete(src._complete)
{}

CGI &CGI::operator=(const CGI &rhs)
{
	CGI tmp(rhs);
	tmp.swap(*this);
	return *this;
}

void CGI::swap(CGI &lhs)
{
	std::swap(_log, lhs._log);
	std::swap(_request, lhs._request);
	std::swap(_scriptPath, lhs._scriptPath);
	std::swap(_interpreter, lhs._interpreter);
	std::swap(_cgiArgv, lhs._cgiArgv);
	std::swap(_cgiEnvp, lhs._cgiEnvp);
	std::swap(_result, lhs._result);
	std::swap(_contentLength, lhs._contentLength);
	std::swap(_cgiFD, lhs._cgiFD);
	std::swap(_complete, lhs._complete);
}
 
CGI::~CGI() {
	std::cout << "CGI destructor called!" << std::endl;
	if (fcntl(_cgiFD, F_GETFD) >= 0)
		close(_cgiFD);
}

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

	std::vector<char*> envp;
	for (const auto& var : _cgiEnvp) {
		envp.push_back(const_cast<char*>(var.c_str()));
	}
	envp.push_back(nullptr);

    if (pipe(pipeServertoCGI) == -1 || pipe(pipeCGItoServer) == -1) {
		_log->logError("Failed to create pipe");
		return;
	}

    _pid = fork();
    if (_pid == -1) {
        close(pipeServertoCGI[READ]);
        close(pipeServertoCGI[WRITE]);
		close(pipeCGItoServer[READ]);
		close(pipeCGItoServer[WRITE]);
        _log->logError("Fork Failed.");
		return;
    }

	if (_pid == 0) {  // Child process
		close(pipeServertoCGI[WRITE]);
		close(pipeCGItoServer[READ]);

		dup2(pipeServertoCGI[READ], STDIN_FILENO);
		close(pipeServertoCGI[READ]);

        dup2(pipeCGItoServer[WRITE], STDOUT_FILENO);
		close(pipeCGItoServer[WRITE]);

        execve(_cgiArgv.data()[0], _cgiArgv.data(), envp.data());
		_log->logError("Execve error.");
		_exit(1);
    }
	else { // Parent process
		close(pipeServertoCGI[READ]);
		close(pipeServertoCGI[WRITE]);
        close(pipeCGItoServer[WRITE]);

		_cgiFD = pipeCGItoServer[READ];
		readCGIfd();
    }
}

int	CGI::readCGIfd()
{
	ssize_t bytesRead;
	std::vector<char> buffer(BUFFSIZE);
	int status = 0;

	if (!waitpid(_pid, &status, WNOHANG))
		return 0;
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
		_log->logError("CGI script exited with non-zero status");
		return 1;
	}
	bytesRead = read(_cgiFD, &buffer[0], BUFFSIZE);
	if (bytesRead < 0) {
		_complete = true;
		return 1;
	}
	else if (bytesRead == 0) {
		close(_cgiFD);
		_complete = true;
		calculateContentLength();
		return 0;
	}
	else {
		_complete = false;
		_result.append(buffer.data(), bytesRead);
		return 0;
	}
}

void	CGI::calculateContentLength()
{
	std::string body;

	if (_result.empty()) {
		_contentLength = 0;
		return;
	}

	size_t start = _result.find(CRLFCRLF);
	if (start != std::string::npos) {
		body = _result.substr(start + 4);
		_contentLength = body.length();
	}
	else
		_contentLength = 0;
}

const size_t		&CGI::get_contentLength() const { return _contentLength; }
const std::string	&CGI::get_result() const { return _result; }
const int			&CGI::get_cgiFD() const { return _cgiFD; }
const bool			&CGI::isComplete() const { return _complete; }
