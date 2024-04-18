#ifndef POLL_H
#define POLL_H

#include <iostream>
#include <vector>
#include <sys/poll.h>
#include <unistd.h>

class Poll{
	private:
	// std::vector<pollfd> fds(2);


	public:
	// start_polling()

};

#endif
// struct	pollfd{
// 	int		fd;
// 	short	events; // events to poll
// 	short	revents; 
// };

// in C++. Here are some of the common events:

// POLLIN: There is data to read.
// POLLPRI: There is urgent data to read.
// POLLOUT: Writing is now possible.
// POLLRDHUP: Stream socket peer closed connection, or shut down writing half of connection. (This flag is Linux-specific.)
// POLLERR: Error condition.
// POLLHUP: Hang up (connection closed).
// POLLNVAL: Invalid request (file descriptor not open).

// 3 = 11
// 2 = 10
// 1 = 01
// 0 = 00

// 2 & 3
// 2 = 10
// 3 = 11
// ------
//     10



