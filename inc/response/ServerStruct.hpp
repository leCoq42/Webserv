#pragma once
#include "ProtoStruct.hpp"
#include "LocationStruct.hpp"

class ServerStruct: public ProtoStruct
{
	private:
	ParserStruct	*_source;

	public:
	int				_serverNum;
	std::string		_id;
	ConfigContent	_port;
	ConfigContent	_host;
	ConfigContent	_names;
	ConfigContent	_root;
	ConfigContent	_location;
	ConfigContent	_errorPage; //list
	ConfigContent	_return; //list
	ConfigContent	_allowMethods;

	ServerStruct();
	ServerStruct(ParserStruct *parser_struct, int nth_server);
	ServerStruct(const ServerStruct &to_copy);
	~ServerStruct();
	ServerStruct	&operator=(const ServerStruct &to_copy);
	void			show_self(void);
};

int	load_in_servers(ParserStruct *PS, std::list<ServerStruct> &server_structs);
