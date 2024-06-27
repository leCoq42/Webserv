#include "ProtoStruct.hpp"
#include "LocationStruct.hpp"

std::list<std::string>	split_content(std::string content)
{
	unsigned long			pos;
	std::list<std::string>	list;
	std::string				word;

	word = content;
	while (word.length())
	{
		pos = word.find_first_not_of(" 	\n");
		if (pos > word.size())
			break ;
		word = word.substr(pos, word.length() - pos);
		pos = word.find_first_of("'");
		if (pos == 0)
		{
			word = word.substr(1, word.length());
			pos = word.find_first_of("'");
		}
		else
			pos = word.find_first_of(" 	\n");
		if (pos > word.size())
			pos = word.size();
		list.push_back(word.substr(0, pos));
		if (pos < word.length())
			pos++;
		word = word.substr(pos, word.length());
	}
	return (list);
}

int	ProtoStruct::getNextContent(ParserItem *current, std::string name, ConfigContent *&variable)
{
	while (current)
	{
		if (!current->getName().compare(name))
		{
			variable = new ConfigContent();
			variable->content_list = split_content(current->getContent());
			if (current->child && !name.compare("location"))
				variable->childs = new LocationStruct(current);
			return (getNextContent(current->next, name, variable->next));//found = true;// return (1); //problematic
		}
		current = current->next;
	}
	return (1);
}

int	ProtoStruct::getContent(std::string name, ConfigContent &variable)
{
	ParserItem		*current;

	current = this->head->child;
	while (current)
	{
		if (!current->getName().compare(name))
		{
			variable.content_list = split_content(current->getContent());
			if (current->child && !name.compare("location"))
				variable.childs = new LocationStruct(current);
			return (getNextContent(current->next, name, variable.next));//found = true;// return (1); //problematic
		}
		current = current->next;
	}
	return (0);
}
