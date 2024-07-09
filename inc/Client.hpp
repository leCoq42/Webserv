#pragma once

#include <memory>
#include <netinet/in.h>
#include "ServerStruct.hpp"
#include "Chunked.hpp"

class Client
{
	public:
		Client();
		~Client();
		Client(Client &src);
		Client &operator =(const Client& client);

	private:
		char	_clientIP[INET_ADDRSTRLEN];
		int		_clientFD;
		ssize_t	_timeOut;
		ssize_t	_lastRequestTime;
		size_t	_numRequests;
		size_t	_maxRequests;
		ServerStruct *_config;
		Chunked	_unchucker;
		char	buffer[1024*100];
		ssize_t	bytesRead = 0;
		std::shared_ptr<Request> _request;
};
