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
	return (*this);
}

LocationStruct::~LocationStruct(void)
{
	std::cout << "Deleting location struct." << std::endl;
}

void	LocationStruct::show_self(void)
{
	std::cout << "	LOCATION STRUCT:" << std::endl;
	if (!this->try_files.content_list.empty())
	{
		std::cout << "	try_files:";
		for (std::string content : this->try_files.content_list)
			std::cout << " {" << content << "}";
		std::cout << std::endl;
	}
	if (!this->index.content_list.empty())
	{
		std::cout << "	index:";
		for (std::string content : this->index.content_list)
			std::cout << " {" << content << "}";
		std::cout << std::endl;
	}
}
