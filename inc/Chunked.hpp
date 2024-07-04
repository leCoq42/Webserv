#pragma once

#include "request.hpp"
#include <fstream>
#include <memory>

//Constructed on firstRequest 

class Chunked
{
	private:
	size_t			_contentLength;		//total length as given in first request
	size_t			_bufferedLength;	//total length saved to file
	std::string		_fileName;			//the file name where there is saved too
	std::string		_boundary;			//the boundary as given in the first request
	public:
	std::shared_ptr<Request>	_firstRequest;	//copy of the first request
	bool			_totalLength;		//false while full length is not reached
	bool			_justStarted;
	Chunked();
	Chunked(std::shared_ptr<Request> first_request);
	Chunked(const Chunked &to_copy);
	Chunked &operator=(const Chunked &to_copy);
	~Chunked();
	void	close_file(); //depending on _totalLength either closes or deletes
	bool	add_to_file(char *buffer, size_t buffer_len); //appends content to bufferFile, true when full length is reached
	std::string	getCombinedBuffer(void);
	std::string	get_fileName(void);
};
