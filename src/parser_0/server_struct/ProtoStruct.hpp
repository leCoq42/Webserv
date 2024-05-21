#pragma once
#include <list>
#include "ConfigContent.hpp"
#include "../text_file_parser/ParserStruct.hpp"

class ProtoStruct
{
	private:
	public:
	ParserItem		*head;
	virtual ~ProtoStruct(void) {};
	virtual int		getContent(std::string name, ConfigContent &variable);
};
