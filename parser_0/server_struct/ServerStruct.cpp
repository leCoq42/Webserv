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
	this->getContent("host", this->host);
	this->getContent("server_name", this->names);
	this->getContent("root", this->root);
	this->getContent("location", this->location);
	std::cout << std::endl;
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
	this->host = to_copy.host;
	this->names = to_copy.names;
	this->location = to_copy.location;
	if (this->location.childs)
	{
		this->location.childs = new LocationStruct;
		*((LocationStruct *)this->location.childs) = *((LocationStruct *)to_copy.location.childs);
	}
	this->id = to_copy.id;
	this->root = to_copy.root;
	return (*this);
}

ServerStruct::~ServerStruct(void)
{
	std::cout << "Deleting server struct." << std::endl;
	if (this->location.childs)
		delete (LocationStruct *)this->location.childs;
	this->nth_server = -1;
}

void	ServerStruct::show_self(void)
{
	std::cout << "server named: " << this->id << " at: "
		<< this->head << std::endl;
	if (!this->port.content_list.empty())
	{
		std::cout << "port:";
		for (std::string content : this->port.content_list)
			std::cout << " {" << content << "}";
		std::cout << std::endl;
	}
	if (!this->host.content_list.empty())
	{
		std::cout << "host:";
		for (std::string content : this->host.content_list)
			std::cout << " {" << content << "}";
		std::cout << std::endl;
	}
	if (!this->names.content_list.empty())
	{
		std::cout << "names:";
		for (std::string content : this->names.content_list)
			std::cout << " {" << content << "}";
		std::cout << std::endl;
	}
	if (!this->root.content_list.empty())
	{
		std::cout << "root:";
		for (std::string content : this->root.content_list)
			std::cout << " {" << content << "}";
		std::cout << std::endl;
	}
	if (!this->location.content_list.empty())
	{
		std::cout << "location:";
		for (std::string content : this->location.content_list)
			std::cout << " {" << content << "}";
		std::cout << std::endl;
	}
	if (this->location.childs)
		((LocationStruct *)this->location.childs)->show_self();
}

int	load_in_servers(ParserStruct *PS, std::list<ServerStruct> &server_structs)
{
	int	n;

	n = 0;
	while (n++ < (*PS).n_servers)
	{
		std::cout << "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Loading in... ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]" << std::endl;
		ServerStruct	add_server = ServerStruct(PS, n);
		server_structs.push_back(add_server);
		// server_structs.back().show_self();
		std::cout << std::endl;
	}
	std::cout << "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]] Loaded all [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[" << std::endl;
	return (1);
}
