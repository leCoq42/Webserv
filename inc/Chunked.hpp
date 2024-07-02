#pragma once

#include "request.hpp"
#include <fstream>
#include <memory>

//Constructed on firstRequest 

class Chunked
{
	private:
	size_t			_contentLength;
	size_t			_bufferedLength;
	bool			_justStarted;
	std::string		_fileName;
	std::string		_boundary;
	public:
	std::shared_ptr<Request>	_firstRequest;
	bool			_totalLength; //false while full length is not reached
	Chunked();
	Chunked(std::shared_ptr<Request> first_request);
	Chunked(const Chunked &to_copy);
	Chunked &operator=(const Chunked &to_copy);
	~Chunked();
	void	close_file(); //depending on _totalLength either closes or deletes
	bool	add_to_file(char *buffer, size_t buffer_len); //appends content to bufferFile, true when full length is reached
	std::string	getCombinedBuffer(void);
};
