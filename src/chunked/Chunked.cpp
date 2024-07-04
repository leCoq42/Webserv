#include "Chunked.hpp"

Chunked::Chunked()
{
	_contentLength = 0;
	_bufferedLength = 0;
	_justStarted = false;
	_totalLength = true;
}

Chunked::Chunked(std::shared_ptr<Request>  first_request): _firstRequest(first_request)
{
	_contentLength = first_request->get_contentLen();
	_bufferedLength = first_request->get_startContentLength();//first_request->get_rawRequest().length();
	_fileName = first_request->get_bufferFile();
	_boundary = first_request->get_boundary();
	std::cout << _bufferedLength << "==" << _contentLength << std::endl;
	if (_bufferedLength >= _contentLength)
		_totalLength = true;
	else
		_totalLength = false;
	_justStarted = false;
}

Chunked::Chunked(const Chunked &to_copy)
{
	*this = to_copy;
}

Chunked::~Chunked()
{
}

Chunked	&Chunked::operator=(const Chunked &to_copy)
{
	_firstRequest = to_copy._firstRequest;
	_justStarted = to_copy._justStarted;
	if (_firstRequest)
	{
		_contentLength = _firstRequest->get_contentLen();
		_bufferedLength = to_copy._bufferedLength;//_firstRequest->get_rawRequest().length();
		_fileName = _firstRequest->get_bufferFile();
		_boundary = _firstRequest->get_boundary();
		if (to_copy._bufferedLength >= to_copy._contentLength)
			_totalLength = true;
		else
			_totalLength = false;
	}
	else
	{
		_contentLength = 0;
		_bufferedLength = 0;
		_fileName = "";
		_totalLength = true;
	}
	return (*this);
}

//Opens specified buffer_file to write the incoming raw request in. First request is done in multipart handler in response.cpp. sequential requests are added here
bool	Chunked::add_to_file(char *buffer, size_t buffer_len)
{
	std::string	buf_str;
	std::ofstream buffer_file;
	size_t		until;

	_justStarted = false;
	buf_str = buffer;
	buffer_file.open(_fileName, std::ios_base::app); // append instead of overwrite
	until = buf_str.find(_boundary);
	_bufferedLength += buffer_len;//buf_str.length();
	if (until != std::string::npos)
		buf_str = buf_str.substr(0, until - 4);
	buffer_file << buf_str;
	std::cout << buffer_len << "written bytes:" << _bufferedLength << "/" << _contentLength << std::endl;
	if (_bufferedLength == _contentLength) //don't match for some reason, headers has to be subtracted
	{
		_totalLength = true;
		return true;
	}
	else if (_bufferedLength > _contentLength)
		return true;
	return false;
}

//Deletes the file if it should be closed because of disconnected client or something but the full body wasn't received yet.
void	Chunked::close_file(void)
{
	_justStarted = false;
	if (!_totalLength)
		std::remove(_fileName.c_str());
}

//depriciated
std::string	Chunked::getCombinedBuffer(void)
{
	return ("");
}

std::string	Chunked::get_fileName(void)
{
	return (_fileName);
}
