#include "cgi.hpp"
#include "defines.hpp"
#include "log.hpp"
#include <cstdlib>
#include <sys/wait.h>
#include <vector>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <signal.h>


CGI::CGI() :
	_request(nullptr), _scriptPath(""), _path(""), _script(""), _interpreter(""), _result(""),
	_contentLength(0), _complete(false)
{
	_pollfdRead.fd = -1;
	_pollfdWrite.fd = -1;
	_pollfdRead.events = 0;
	_pollfdWrite.events = 0;
}


CGI::CGI(const std::shared_ptr<Request> &request, const std::filesystem::path &scriptPath, const std::string &interpreter, std::shared_ptr<Log> log) :
	_log(log), _request(request), _scriptPath(scriptPath), _path(scriptPath), _script(""), _interpreter(interpreter), _result(""),
	_contentLength(0), _complete(false), _pid(0)
{
	_pollfdRead.fd = -1;
	_pollfdWrite.fd = -1;
	_pollfdRead.events = 0;
	_pollfdWrite.events = 0;
	parseCGI();
	executeScript();
	if (_complete)
		calculateContentLength();
}

CGI::CGI(const CGI &src) :
	_log(src._log), _request(src._request), _scriptPath(src._scriptPath), _path(src._path), _script(src._script), _interpreter(src._interpreter),
	_cgiArgv(src._cgiArgv), _cgiEnvp(src._cgiEnvp), _result(src._result),
	_contentLength(src._contentLength), _complete(src._complete)
{
	_pollfdRead.fd = src._pollfdRead.fd;
	_pollfdWrite.fd = src._pollfdWrite.fd;
	_pollfdRead.events = src._pollfdRead.events;
	_pollfdWrite.events = src._pollfdWrite.events;
}

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
	std::swap(_path, lhs._path);
	std::swap(_interpreter, lhs._interpreter);
	std::swap(_cgiArgv, lhs._cgiArgv);
	std::swap(_cgiEnvp, lhs._cgiEnvp);
	std::swap(_result, lhs._result);
	std::swap(_contentLength, lhs._contentLength);
	std::swap(_pollfdRead.fd, lhs._pollfdRead.fd);
	std::swap(_pollfdWrite.fd, lhs._pollfdWrite.fd);
	std::swap(_pollfdRead.events, lhs._pollfdRead.events);
	std::swap(_pollfdWrite.events, lhs._pollfdWrite.events);
	std::swap(_complete, lhs._complete);
	std::swap(_script, lhs._script);
}
 
CGI::~CGI() {
	int	status;
	
	if (!waitpid(_pid, &status, WNOHANG))
		kill(_pid, SIGKILL);
	if (_pollfdRead.fd != -1)
		close(_pollfdRead.fd);
	if (_pollfdWrite.fd != -1)
		close(_pollfdWrite.fd);
}

void CGI::parseCGI()
{
	_script = _scriptPath.filename().c_str();
	init_envp();
	if (!_interpreter.empty())
		_cgiArgv.push_back(const_cast<char *>(_interpreter.c_str()));
	_cgiArgv.push_back(const_cast<char *>(_script.c_str()));
	_cgiArgv.push_back(NULL);
	if (!_request->get_requestArgs().empty()) {
		for (auto it: _request->get_requestArgs())
			add_to_envp(it.first, it.second, "HTTP_");
	}
}

void CGI::init_envp()
{
	add_to_envp("GATEWAY_INTERFACE", "CGI/1.1", "");
	add_to_envp("REQUEST_METHOD", _request->get_requestMethod(), "");
	add_to_envp("REDIRECT_STATUS", "true", "");
	add_to_envp("SCRIPT_FILENAME", _script, "");
	add_to_envp("SCRIPT_NAME", _script, "");
	if (_request->get_requestMethod() == "GET")
		add_to_envp("QUERY_STRING", _request->_argStr.c_str(), "");
	for (const auto &[key, value] : _request->get_headers())
    	add_to_envp(key, value, "");
}

bool CGI::add_to_envp(std::string key, std::string value, std::string prefix)
{
	std::string tmp;

	std::replace(key.begin(), key.end(), '-', '_');
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
	_bytesWritten = 0;

    _pid = fork();
    if (_pid == -1) {
        close(pipeServertoCGI[READ]);
        close(pipeServertoCGI[WRITE]);
		close(pipeCGItoServer[READ]);
		close(pipeCGItoServer[WRITE]);
        _log->logError("Fork Failed.");
		return;
    }

	if (_pid == 0) {// Child process
		chdir(_path.remove_filename().c_str());
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
		close(pipeCGItoServer[WRITE]);
		if (!_request->get_body().length())
		{
			_pollfdWrite.fd = pipeServertoCGI[WRITE];
			close(_pollfdWrite.fd);
			_pollfdWrite.fd = -1;
		}
		else
		{
			_pollfdWrite.fd = pipeServertoCGI[WRITE];
			_pollfdWrite.events = POLLOUT;
		}
		_pollfdRead.fd = pipeCGItoServer[READ];
		_pollfdRead.events = POLLIN;

    }
}

int	CGI::readCGIfd()
{
	ssize_t bytesRead;
	std::vector<char> buffer(BUFFSIZE);

	if (_pollfdRead.fd > 0)
		bytesRead = read(_pollfdRead.fd, &buffer[0], BUFFSIZE);
	else
		bytesRead = 0;
	if (bytesRead < 0) {
		if (_pollfdRead.fd > 0)
			close(_pollfdRead.fd);
		_pollfdRead.fd = -1;
		_complete = true;
		calculateContentLength();
		return 1;
	}
	else if (bytesRead == 0) {
		if (_pollfdRead.fd > 0)
			close(_pollfdRead.fd);
		_pollfdRead.fd = -1;
		_complete = true;
		calculateContentLength();
	}
	else {
		_complete = false;
		_result.append(buffer.data(), bytesRead);
	}
	return 0;
}

int	CGI::writeCGIfd()
{
	size_t	to_write;

	to_write = _request->get_body().length() - _bytesWritten;
	if (to_write > BUFFSIZE)
		to_write = BUFFSIZE;
	size_t bytes_written = write(_pollfdWrite.fd, _request->get_body().data() + _bytesWritten, to_write);
	_bytesWritten += to_write;
	if (_bytesWritten == _request->get_body().length())
	{
		_pollfdWrite.events = 0;
		close(_pollfdWrite.fd);
		_pollfdWrite.fd = -1;
		return (1);
	}
	if (bytes_written < 0) {
		close(_pollfdWrite.fd);
		kill(_pid, SIGKILL);
		_pollfdRead.fd = -1;
		_pollfdWrite.fd = -1;
		_log->logError("Failed to write to pipe");
		return (2);
	}
	return (0);
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
pollfd				&CGI::get_pollfdRead() { return _pollfdRead; }
pollfd				&CGI::get_pollfdWrite() { return _pollfdWrite; }
const bool			&CGI::isComplete() const { return _complete; }
