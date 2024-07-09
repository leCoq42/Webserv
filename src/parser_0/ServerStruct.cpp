#include "ServerStruct.hpp"

ServerStruct::ServerStruct(void)
{
	this->_nthServer = -1;
	this->_source = 0;
	this->head = 0;
}

ServerStruct::ServerStruct(ParserStruct *parser_struct, int nth_server)
{
	this->_nthServer = nth_server;
	this->_source = parser_struct;
	this->head = parser_struct->go_to_nth(nth_server);
	this->_id = this->head->getContent();
	this->getContent("listen", this->_port);
	this->_port.combine();
	this->getContent("host", this->_host);
	this->getContent("server_name", this->_names);
	this->getContent("root", this->_root);
	this->getContent("location", this->_location);
	this->getContent("error_page", this->_errorPage);
	this->getContent("return", this->_return);
	this->getContent("allow_methods", this->_allowMethods);
	//std::cout << std::endl;
}

ServerStruct::ServerStruct(const ServerStruct &to_copy)
{
	*this = to_copy;
}

ServerStruct	&ServerStruct::operator=(const ServerStruct &to_copy)
{
	this->_nthServer = to_copy._nthServer;
	this->_source = to_copy._source;
	this->head = to_copy.head;
	this->_port = to_copy._port;
	this->_names = to_copy._names;
	this->_location = to_copy._location;
	this->_allowMethods = to_copy._allowMethods;
	this->_id = to_copy._id;
	this->_root = to_copy._root;
	this->_return = to_copy._return;
	this->_errorPage = to_copy._errorPage;
	return (*this);
}

ServerStruct::~ServerStruct(void)
{
	//std::cout << "Deleting server struct." << std::endl;
	// if (this->location.childs)
	// 	delete (LocationStruct *)this->location.childs;
	this->_nthServer = -1;
}

void	ServerStruct::show_self(void)
{
	std::cout << "server named: " << this->_id << " at: "
		<< this->head << std::endl;
	this->_port.show_part("port:");
	this->_names.show_part("names:");
	this->_root.show_part("root:");
	this->_location.show_part("location:");
	this->_return.show_part("return:");
	this->_errorPage.show_part("error_page:");
	this->_allowMethods.show_part("allow_methods:");
}

int	load_in_servers(ParserStruct *PS, std::list<ServerStruct> &server_structs)
{
	size_t	n;

	n = 0;
	while (n++ < (*PS).get_nServers())
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
