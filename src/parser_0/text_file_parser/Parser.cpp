#include "Parser.hpp"

Parser::Parser(void)
{
	this->level = 0;
	this->old_level = 0;
	std::cout << "Empty parser constraints, very handy......" << std::endl;
}

Parser::Parser(std::string comment, std::string comment_end, std::string to_child, std::string to_parent, std::string ignore, std::string encapsulators, std::string name_end, std::string line_end)
{
	this->level = 0;
	this->old_level = 0;
	this->comment = comment;
	this->comment_end = comment_end;
	this->to_child = to_child;
	this->to_parent = to_parent;
	this->ignore = ignore;
	this->encapsulators = encapsulators;
	this->name_end = name_end;
	this->line_end = line_end;
}

Parser::~Parser()
{
	std::cout << "bye bye parser" << std::endl;
}

int		Parser::encapsulator_skip(char *buffer, unsigned long &i)
{
	if (this->encapsulators.find(*(buffer + i++)) != std::string::npos)
			while (this->encapsulators.find(*(buffer + i++)) == std::string::npos)
				if (!(*(buffer + i)))
					return (0);
	return (1);
}

void	Parser::write_over_comments(char *buffer, unsigned long start, unsigned long &i, unsigned long &buffer_len)
{
	unsigned long	r = 0;

	while (i + r < buffer_len - 1)
	{
		*(buffer + start + r) = *(buffer + i + r);
		r++;
	}
	*(buffer + start + r) = 0;
	buffer_len -= i - start;
}

void	Parser::remove_comments(char *buffer, unsigned long &start, unsigned long &buffer_len)
{
	unsigned long	i = start;

	while (this->comment.find(*(buffer + i)) == std::string::npos && *(buffer + i))
		if (!this->encapsulator_skip(buffer, i))
			return ;
	if (i < buffer_len)
	{
		start = i;
		while (this->comment_end.find(*(buffer + i)) == std::string::npos && *(buffer + i))
			i++;
		this->write_over_comments(buffer, start, i, buffer_len);
		if (i < buffer_len - 1)
			this->remove_comments(buffer, start, buffer_len);
	}
}

int	Parser::find_obj(char *buffer, unsigned long start, unsigned long buffer_len, int &end)
{
	unsigned long	i = start;

	while (this->ignore.find(*(buffer + i)) != std::string::npos && i < buffer_len)
		i++;
	while (this->to_parent.find(*(buffer + i)) != std::string::npos)
	{
		this->PS.to_parent();
		this->level--;
		this->old_level--;
		i++;
		while (this->ignore.find(*(buffer + i)) != std::string::npos && i < buffer_len)
			i++;
	}
	if (!(i < buffer_len - 1))
	{
		end = 1;
		return (0);
	}
	start = i;
	this->name = buffer + i;
	while (this->name_end.find(*(buffer + i)) == std::string::npos && i < buffer_len)
		i++;
	if (!(i < buffer_len - 1))
	{
		end = 1;
		return (0);
	}
	*(buffer + i++) = 0;
	this->content = buffer + i;
	while (this->line_end.find(*(buffer + i)) == std::string::npos && this->to_child.find(*(buffer + i)) == std::string::npos && i < buffer_len)
		if (!this->encapsulator_skip(buffer, i))
			return (0);
	if (this->to_child.find(*(buffer + i)) != std::string::npos)
		this->level++;
	if (i < buffer_len)
		*(buffer + i++) = 0;
	return (i);
}

int Parser::add_obj(void)
{
	if (!this->PS.add(this->name, this->content))
		return (0);
	if (this->level > this->old_level)
	{
		this->PS.to_child();
		this->old_level++;
	}
	if (this->level != this->old_level)
		return (0);
	return (1);
}

int	Parser::parse_content_to_struct(char *buffer, unsigned long buffer_len)
{
	std::string		file_content;
	unsigned long	i = 0;
	int				end = 0;

	file_content = buffer;
	this->remove_comments(buffer, i, buffer_len);
	std::cout << "+++++++++++" << buffer_len << std::endl << buffer << std::endl << "+++++++++++comments removed" << std::endl;
	i = 0;
	while (i < buffer_len - 1 && !end)
	{
		i = find_obj(buffer, i, buffer_len, end);
		if (i)
			if (!add_obj())
				return (0);
	}
	std::cout << "+++++++++++" << this->level << this->old_level << std::endl;
	this->PS.display(true);
	if (this->old_level)
	{
		std::cout << "LEVEL ISSUE: " << this->old_level << std::endl;
		return (0);
	}
	this->PS.count_servers();
	return (1);
}
