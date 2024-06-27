#include "ConfigContent.hpp"
#include "LocationStruct.hpp"

ConfigContent::ConfigContent(void)
{
	this->childs = NULL;
	next = NULL;
}

ConfigContent::ConfigContent(const ConfigContent &to_copy)
{
	next = NULL;
	childs = NULL;
	*this = to_copy;
}

ConfigContent &ConfigContent::operator=(const ConfigContent &to_copy)
{
	if (childs)
		delete (LocationStruct *)childs;
	if (to_copy.childs)
		this->childs = new LocationStruct(*(LocationStruct *)to_copy.childs);
	else
		this->childs = NULL;
	if (next)
		delete next;
	if (to_copy.next)
		next = new ConfigContent(*to_copy.next);
	else
		next = NULL;
	this->content_list = to_copy.content_list;
	return (*this);
}

ConfigContent::~ConfigContent()
{
	if (next)
		delete next;
	if (childs)
		delete (LocationStruct *)childs;
}

void	ConfigContent::combine(void)
{
	ConfigContent	*variable;

	variable = next;
	while (variable)
	{
		for (std::string content : variable->content_list)
			content_list.push_back(content);
		variable = variable->next;
	}
}

void	ConfigContent::show_part(std::string name)
{
	if (content_list.empty())
		return ;
	std::cout << name;
	for (std::string content : content_list)
			std::cout << " {" << content << "}";
	std::cout << std::endl;
	if (!name.compare("location:"))
		if (childs)
			((LocationStruct *)childs)->show_self();
	if (next)
		next->show_part(name);
}
