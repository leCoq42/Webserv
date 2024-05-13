#pragma once
#include "ParserItem.hpp"

class ParserStruct
{
	private:
	ParserItem	*head;
	ParserItem	*current;
	int			add_as_child;
	public:
	int		n_servers;
	ParserStruct(void);
	~ParserStruct();
	void	display(bool with_content);
	int		add(std::string name, std::string content);
	void	to_child(void);
	void	to_parent(void);
	int		count_servers(void);
	ParserItem	*go_to_nth(int n);
};
