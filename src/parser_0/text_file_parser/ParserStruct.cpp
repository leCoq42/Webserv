#include "ParserStruct.hpp"

ParserStruct::ParserStruct(void)
{
	this->head = 0;
	this->current = 0;
	this->add_as_child = 0;
	this->n_servers = 0;
}

ParserStruct::~ParserStruct()
{
	std::cout << "delete parser struct" << std::endl;
	delete this->head;
	this->head = 0;
}

void	ParserStruct::display(bool with_content)
{
	std::cout << "_____ParserStruct_____" << std::endl;
	if (this->head)
		this->head->display(0, with_content);
	std::cout << "______________________" << std::endl;
}

void	ParserStruct::to_child(void)
{
	if (this->current && this->current->child)
		this->current = this->current->child;
	else
		this->add_as_child = 1;
}

void	ParserStruct::to_parent(void)
{
	if (this->add_as_child)
		this->add_as_child = 0;
	else if (this->current->parent)
		this->current = this->current->parent;
}

int		ParserStruct::add(std::string name, std::string content)
{
	ParserItem	**dest;

	if (this->add_as_child)
	{
		dest = &this->current->child;
		*dest = new ParserItem(name, content, this->current);
		this->add_as_child = 0;
	}
	else if (this->current)
	{
		dest = &this->current->next;
		*dest = new ParserItem(name, content, this->current->parent);
	}
	else
	{
		dest = &this->head;
		*dest = new ParserItem(name, content, 0);
	}
	if (!*dest)
		return (0);
	this->current = *dest;
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

	this->n_servers = 0;
	current = head;
	while (current)
	{
		if (!current->getName().compare("server"))
			this->n_servers++;
		if (current->getName().compare("server") && current->child)
			current = current->child;
		else
			current = get_new_up(current);
	}
	return (this->n_servers);
}

ParserItem	*ParserStruct::go_to_nth(int n)
{
	ParserItem	*current;

	current = head;
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
