#include "ServerStruct.hpp"
#include "defines.hpp"

ServerStruct::ServerStruct(void)
{
	this->_serverNum = -1;
	this->_source = 0;
	this->head = 0;
}

ServerStruct::ServerStruct(ParserStruct *parser_struct, int nth_server)
{
	this->_serverNum = nth_server;
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
	this->getContent("client_max_body_size", this->_clientMaxBodySize);
	if (this->_clientMaxBodySize.content_list.empty())
		this->_clientMaxBodySize.content_list.push_back(std::to_string(DEFUALT_CLIENT_MAX_BODY_SIZE));
}

ServerStruct::ServerStruct(const ServerStruct &to_copy)
{
	*this = to_copy;
}

ServerStruct	&ServerStruct::operator=(const ServerStruct &to_copy)
{
	this->_serverNum = to_copy._serverNum;
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
	this->_clientMaxBodySize = to_copy._clientMaxBodySize;
	return (*this);
}

ServerStruct::~ServerStruct(void)
{
	this->_serverNum = -1;
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
	this->_clientMaxBodySize.show_part("client_max_body_size:");
}

int	empty_locations(ServerStruct	&add_server)
{
	ConfigContent	*current;

	if (!add_server._return.content_list.empty())
		return (0);
	if (add_server._root.content_list.empty() || add_server._root.content_list.size() > 1)
		return (1);
	if (add_server._root.content_list.back() == "")
		return (1);
	current = &add_server._location;
	while (current)
	{
		if (current->content_list.empty())
			return (1);
		current = current->next;
	}
	return (0);
}

int	load_in_servers(ParserStruct *PS, std::list<ServerStruct> &server_structs)
{
	size_t	n;

	n = 0;
	while (n++ < (*PS).get_nServers())
	{
		ServerStruct	add_server = ServerStruct(PS, n);
		if (empty_locations(add_server))
			return (0);
		server_structs.push_back(add_server);
	}
	return (1);
}

//no double in same server
int	double_ports(std::list<ServerStruct> &server_structs)
{
	std::list<int>	all_ports;

	for (std::list<ServerStruct>::iterator server = server_structs.begin(); server != server_structs.end();)
	{
		for (std::list<std::string>::iterator port_str = server->_port.content_list.begin(); port_str != server->_port.content_list.end();)
		{
			try
			{
				all_ports.push_back(std::stoi(*port_str));
			}
			catch (std::exception &e)
			{
				throw(std::runtime_error("Port is not an number."));
			}
			port_str++;
		}
		for (std::list<int>::iterator port = all_ports.begin(); port != all_ports.end();)
		{
			for (std::list<int>::iterator compare_port = port; compare_port != all_ports.end();)
			{
				compare_port++;
				if (compare_port != all_ports.end() && *compare_port == *port)
					return (1);
			}
			port++;
		}
		if (all_ports.empty())
			throw (std::runtime_error("No ports selected"));// return (1);
		server++;
	}
	return (0);
}

//never double
// int	double_ports(std::list<ServerStruct> &server_structs)
// {
// 	std::list<int>	all_ports;

// 	for (std::list<ServerStruct>::iterator server = server_structs.begin(); server != server_structs.end();)
// 	{
// 		for (std::list<std::string>::iterator port_str = server->_port.content_list.begin(); port_str != server->_port.content_list.end();)
// 		{
// 			try
// 			{
// 				all_ports.push_back(std::stoi(*port_str));
// 			}
// 			catch (std::exception &e)
// 			{
// 				throw(std::runtime_error("Port is not an number."));
// 			}
// 			port_str++;
// 		}
// 		server++;
// 	}
// 	for (std::list<int>::iterator port = all_ports.begin(); port != all_ports.end();)
// 	{
// 		for (std::list<int>::iterator compare_port = port; compare_port != all_ports.end();)
// 		{
// 			compare_port++;
// 			if (compare_port != all_ports.end() && *compare_port == *port)
// 				return (1);
// 		}
// 		port++;
// 	}
// 	if (all_ports.empty())
// 		throw (std::runtime_error("No ports selected"));// return (1);
// 	return (0);
// }
