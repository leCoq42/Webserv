#pragma once

#include "ParserStruct.hpp"

class Parser
{
	private:
	int			level;
	int			old_level;
	char		*name;			//payload name
	char		*content;		//payload
	std::string	comment;		//start of ignorable comment
	std::string	comment_end;	//end of ignorable comment
	std::string	to_child;		//jump in hierarchy
	std::string	to_parent;		//jump in hierarchy
	std::string	ignore;			//human readability helpers 
	std::string	encapsulators;	//encapsulating for syntax ignore
	std::string	name_end;		//syntax from name to content, whitespace = etc
	std::string	line_end;		//syntax end of content, \n ; etc
	int	encapsulator_skip(char *buffer, unsigned long &i);
	int	find_obj(char *buffer, unsigned long start, unsigned long buffer_len, int &end);
	int	add_obj(void);
	void	write_over_comments(char *buffer, unsigned long start, unsigned long &i, unsigned long &buffer_len);
	void	remove_comments(char *buffer, unsigned long &start, unsigned long &buffer_len);
	public:
	ParserStruct	PS;
	Parser(void);
	Parser(std::string comment, std::string comment_end, std::string to_child, std::string to_parent, std::string ignore, std::string encapsulators, std::string name_end, std::string line_end);
	~Parser();
	int	parse_content_to_struct(char *buffer, unsigned long buffer_len);
};
