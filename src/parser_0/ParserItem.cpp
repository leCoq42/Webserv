#include "ParserItem.hpp"

ParserItem::ParserItem(void)
{
	this->next = 0;
	this->child = 0;
	this->parent = 0;
}

ParserItem::ParserItem(std::string name, std::string content, ParserItem *parent)
{
	this->next = 0;
	this->child = 0;
	this->parent = parent;
	this->name = name;
	this->content = content;
}

ParserItem::ParserItem(ParserItem &to_copy)
{
	this->next = 0;
	this->child = 0;
	this->parent = to_copy.parent;
	this->name = to_copy.name;
	this->content = to_copy.content;
}

ParserItem	&ParserItem::operator=(ParserItem &to_copy)
{
	if (this->next)
		delete this->next;
	if (this->child)
		delete this->child;
	this->next = to_copy.next;
	this->child = to_copy.child;
	this->parent = to_copy.parent;
	this->name = to_copy.name;
	this->content = to_copy.content;
	return (*this);
}

ParserItem::~ParserItem()
{
	this->name = "";
	this->content = "";
	if (this->child)
		delete this->child;
	if (this->next)
		delete this->next;
}

void	ParserItem::display(int depth, bool with_content)
{
	int	depth_copy = depth;

	while (depth_copy--)
		std::cout << "	";
	std::cout << this->name;
	if (with_content)
		std::cout << "_:_" << this->content;
	std::cout << std::endl;
	if (this->child)
		this->child->display(depth + 1, with_content);
	if (this->next)
		this->next->display(depth, with_content);
}

std::string ParserItem::getName(void)
{
	return (this->name);
}

std::string ParserItem::getContent(void)
{
	return (this->content);
}
