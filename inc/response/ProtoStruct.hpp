#pragma once
#include <list>
#include "ConfigContent.hpp"
#include "ParserStruct.hpp"

class ProtoStruct
{
	private:
	public:
	ParserItem		*head;
	virtual ~ProtoStruct(void) {};
	virtual int		getNextContent(ParserItem *current, std::string name, ConfigContent *&variable);
	virtual int		getContent(std::string name, ConfigContent &variable);
};
