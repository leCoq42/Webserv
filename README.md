# Webserv
**This project is about writing a HTTP server in c++ which will be tested in an actual browser.**


The server must be able to handle multiple ports, serve static websites, accept GET, POST, and DELETE methods, allow file uploads, and be non-blocking using only one poll() (or equivalent) for all I/O operations. Additionally, the server must accurately handle HTTP response status codes, have default error pages, and be compatible with HTTP 1.1 standards like NGINX.

The configuration file allows users to customize server settings such as port, host, server names, error pages, route configurations, directory listing, default files, and CGI execution. Notably, CGI execution should be handled with the full path as PATH_INFO and must work with one CGI, such as PHP or Python.

For MacOS, fcntl() can be used to implement non-blocking behavior, with specific flags allowed.

The bonus part includes additional features like support for cookies and session management, as well as handling multiple CGI.


Program name:        <pre>Webserv</pre>
Makefile:            <pre>all, clean, fclean, re, bonus</pre>
Arguments:           <pre>A configuration file</pre>
External functions:  <pre>Everything in C++11</pre>
External libraties:  <pre>Any external library and Boost libraries are forbidden</pre>


This project was accomplished by [leCoq42](https://github.com/leCoq42), @Joost-L & @Chavert-ter-Maat
