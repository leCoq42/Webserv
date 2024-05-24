#include "ConfigContent.hpp"

ConfigContent::ConfigContent(void)
{
	this->childs = 0;
}

ConfigContent::ConfigContent(const ConfigContent &to_copy)
{
	*this = to_copy;
}

ConfigContent &ConfigContent::operator=(const ConfigContent &to_copy)
{
	this->childs = to_copy.childs;
	this->content_list = to_copy.content_list;
	return (*this);
}

ConfigContent::~ConfigContent()
{
}
