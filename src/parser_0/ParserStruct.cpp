#include "ParserStruct.hpp"

ParserStruct::ParserStruct(void) : _head(0), _current(0), _addAsChild(0), _nServers(0)
{}

ParserStruct::~ParserStruct()
{
	delete _head;
	_head = 0;
}

void	ParserStruct::display(bool with_content)
{
	std::cout << "_____ParserStruct_____" << std::endl;
	if (this->_head)
		this->_head->display(0, with_content);
	std::cout << "______________________" << std::endl;
}

void	ParserStruct::to_child(void)
{
	if (this->_current && this->_current->child)
		this->_current = this->_current->child;
	else
		this->_addAsChild = 1;
}

void	ParserStruct::to_parent(void)
{
	if (this->_addAsChild)
		this->_addAsChild = 0;
	else if (this->_current->parent)
		this->_current = this->_current->parent;
}

int		ParserStruct::add(std::string name, std::string content)
{
	ParserItem	**dest;

	if (this->_addAsChild)
	{
		dest = &this->_current->child;
		*dest = new ParserItem(name, content, this->_current);
		this->_addAsChild = 0;
	}
	else if (this->_current)
	{
		dest = &this->_current->next;
		*dest = new ParserItem(name, content, this->_current->parent);
	}
	else
	{
		dest = &this->_head;
		*dest = new ParserItem(name, content, 0);
	}
	if (!*dest)
		return (0);
	this->_current = *dest;
	return (1);
}

ParserItem *get_new_up(ParserItem *current)
{
	if (current)
	{
		if (current->next)
			return (current->next);
		return (get_new_up(current->parent));
	}
	return (current);
}

int		ParserStruct::count_servers(void)
{
	ParserItem	*current;

	this->_nServers = 0;
	current = _head;
	while (current)
	{
		if (!current->getName().compare("server"))
			this->_nServers++;
		if (current->getName().compare("server") && current->child)
			current = current->child;
		else
			current = get_new_up(current);
	}
	return (this->_nServers);
}

ParserItem	*ParserStruct::go_to_nth(int n)
{
	ParserItem	*current;

	current = _head;
	while (current)
	{
		if (!current->getName().compare("server"))
		{
			if (!--n)
				return (current);
		}
		if (current->getName().compare("server") && current->child)
			current = current->child;
		else
			current = get_new_up(current);
	}
	return (current);
}

size_t ParserStruct::get_nServers()
{
	return _nServers;
}
