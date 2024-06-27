#pragma once
#include "ProtoStruct.hpp"

class LocationStruct: public ProtoStruct
{
	private:
	public:
	std::string				id;
	ConfigContent	try_files;
	ConfigContent	host;
	ConfigContent	index;
	ConfigContent	autoindex;
	ConfigContent	_return;
	ConfigContent	root;
	ConfigContent	allow_methods;
	LocationStruct(void);
	LocationStruct(ParserItem *head);
	LocationStruct(const LocationStruct &to_copy);
	LocationStruct	&operator=(const LocationStruct &to_copy);
	~LocationStruct(void);
	void	show_self(void);
};
