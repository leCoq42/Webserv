#include "ServerStruct.hpp"

ServerStruct::ServerStruct(void)
{
	this->nth_server = -1;
	this->source = 0;
	this->head = 0;
}

ServerStruct::ServerStruct(ParserStruct *parser_struct, int nth_server)
{
	this->nth_server = nth_server;
	this->source = parser_struct;
	this->head = parser_struct->go_to_nth(nth_server);
	this->id = this->head->getContent();
	this->getContent("listen", this->port);
	this->port.combine();
	this->getContent("host", this->host);
	this->getContent("server_name", this->names);
	this->getContent("root", this->root);
	this->getContent("location", this->location);
	this->getContent("error_page", this->error_page);
	this->getContent("return", this->_return);
	this->getContent("allow_methods", this->allow_methods);
	//std::cout << std::endl;
}

ServerStruct::ServerStruct(const ServerStruct &to_copy)
{
	*this = to_copy;
}

ServerStruct	&ServerStruct::operator=(const ServerStruct &to_copy)
{
	this->nth_server = to_copy.nth_server;
	this->source = to_copy.source;
	this->head = to_copy.head;
	this->port = to_copy.port;
	this->names = to_copy.names;
	this->location = to_copy.location;
	this->allow_methods = to_copy.allow_methods;
	this->id = to_copy.id;
	this->root = to_copy.root;
	this->_return = to_copy._return;
	this->error_page = to_copy.error_page;
	return (*this);
}

ServerStruct::~ServerStruct(void)
{
	//std::cout << "Deleting server struct." << std::endl;
	// if (this->location.childs)
	// 	delete (LocationStruct *)this->location.childs;
	this->nth_server = -1;
}

void	ServerStruct::show_self(void)
{
	std::cout << "server named: " << this->id << " at: "
		<< this->head << std::endl;
	this->port.show_part("port:");
	this->names.show_part("names:");
	this->root.show_part("root:");
	this->location.show_part("location:");
	this->_return.show_part("return:");
	this->error_page.show_part("error_page:");
	this->allow_methods.show_part("allow_methods:");
}

int	load_in_servers(ParserStruct *PS, std::list<ServerStruct> &server_structs)
{
	int	n;

	n = 0;
	while (n++ < (*PS).n_servers)
	{
		//std::cout << "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Loading in... ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]" << std::endl;
		ServerStruct	add_server = ServerStruct(PS, n);
		server_structs.push_back(add_server);
		// server_structs.back().show_self();
		//std::cout << std::endl;
	}
	//std::cout << "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]] Loaded all [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[" << std::endl;
	return (1);
}
