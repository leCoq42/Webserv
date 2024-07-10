#pragma once

#include <iostream>

class ParserItem
{
	private:
	std::string	name;
	std::string	content;
	public:
	ParserItem	*next;
	ParserItem	*child;
	ParserItem	*parent;
	ParserItem(void);
	ParserItem(std::string name, std::string content, ParserItem *parent);
	ParserItem(ParserItem &to_copy);
	ParserItem &operator=(ParserItem &to_copy);
	~ParserItem();
	void	display(int depth, bool with_content);
	std::string	getName(void);
	std::string getContent(void);
};
