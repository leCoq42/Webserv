#include "LocationStruct.hpp"

LocationStruct::LocationStruct(void)
{
	this->head = 0;
}

LocationStruct::LocationStruct(ParserItem *head)
{
	this->head = head;
	this->id = this->head->getContent();
	this->getContent("try_files", this->try_files);
	this->getContent("index", this->index);
	this->getContent("autoindex", this->autoindex);
	this->getContent("return", this->_return);
	this->getContent("root", this->root);
	this->getContent("allow_methods", this->allow_methods);
}

LocationStruct::LocationStruct(const LocationStruct &to_copy)
{
	*this = to_copy;
}

LocationStruct	&LocationStruct::operator=(const LocationStruct &to_copy)
{
	this->head = to_copy.head;
	this->id = to_copy.id;
	this->try_files = to_copy.try_files;
	this->index = to_copy.index;
	this->autoindex = to_copy.autoindex;
	this->_return = to_copy._return;
	this->root = to_copy.root;
	this->allow_methods = to_copy.allow_methods;
	return (*this);
}

LocationStruct::~LocationStruct(void)
{
}

void	LocationStruct::show_self(void)
{
	std::cout << "	LOCATION STRUCT:" << std::endl;
	this->try_files.show_part("	try_files:");
	this->index.show_part("	index:");
	this->autoindex.show_part("	autoindex:");
	this->_return.show_part("	return:");
	this->root.show_part("	root:");
	this->allow_methods.show_part("	allow_methods:");
}
