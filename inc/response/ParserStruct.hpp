#pragma once
#include "ParserItem.hpp"

class ParserStruct
{
	private:
	ParserItem	*_head;
	ParserItem	*_current;
	int			_addAsChild;
	size_t		_nServers;

	public:
	ParserStruct(void);
	~ParserStruct();
	void	display(bool with_content);
	int		add(std::string name, std::string content);
	void	to_child(void);
	void	to_parent(void);
	int		count_servers(void);
	ParserItem	*go_to_nth(int n);
	size_t	get_nServers();
};
