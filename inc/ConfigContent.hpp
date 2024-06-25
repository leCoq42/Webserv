#pragma once
#include <list>
#include <iostream>

class ConfigContent
{
	public:
	std::list<std::string>	content_list;
	void					*childs;
	ConfigContent(void);
	ConfigContent(const ConfigContent &to_copy);
	ConfigContent	&operator=(const ConfigContent &to_copy);
	~ConfigContent();
};
