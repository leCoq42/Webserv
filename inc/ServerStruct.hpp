#pragma once
#include "ProtoStruct.hpp"
#include "LocationStruct.hpp"

class ServerStruct: public ProtoStruct
{
	private:
	ParserStruct	*source;
	public:
	int				nth_server;
	std::string		id;
	ConfigContent	port;
	ConfigContent	host;
	ConfigContent	names;
	ConfigContent	root;
	ConfigContent	location;
	ConfigContent	error_page; //list
	ConfigContent	_return; //list
	ConfigContent	allow_methods;

	ServerStruct();
	ServerStruct(ParserStruct *parser_struct, int nth_server);
	ServerStruct(const ServerStruct &to_copy);
	~ServerStruct();
	ServerStruct	&operator=(const ServerStruct &to_copy);
	void			show_self(void);
};

int	load_in_servers(ParserStruct *PS, std::list<ServerStruct> &server_structs);
