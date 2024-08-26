#include "ClientConnection.hpp"
#include "log.hpp"
#include "Parser.hpp"
#include "ServerConnection.hpp"
#include "ServerStruct.hpp"
#include "fileHandler.hpp"
#include "signals.hpp"
#include <iostream>
#include <memory>

#define STD_CONFIG "basic_config.txt"

void error_exit(int error_code) {
	if (error_code == 1)
		std::cerr << "no file given" << std::endl;
	else if (error_code == 2)
		std::cerr << "file loading error" << std::endl;
	else if (error_code == 3)
		std::cerr << "invalid config file" << std::endl;
	else if (error_code == 4)
		std::cerr << "loading server struct went wrong" << std::endl;
	else if (error_code == 5)
		std::cerr << "Duplicate ports detected" << std::endl;
}

int parse(Parser *parser, std::list<ServerStruct> *server_structs,
           char **buffer, std::string config_file_name)
{
	int file_len;

	try
	{
		if (load_file_to_buff(config_file_name.c_str(), buffer, &file_len))
			return (2);
		else if (!parser->parse_content_to_struct(*buffer, file_len))
			return (3);
		else if (!load_in_servers(&parser->PS, *server_structs))
			return (4);
		else if (double_ports(*server_structs))
				return (5);
		else
		{
			#ifdef DEBUG
			std::cout << parser->PS.get_nServers() << std::endl;
			if (!server_structs->empty()) {
					for (ServerStruct server : *server_structs) {
						std::cout << "-------------------------------------" << std::endl;
						server.show_self();
						std::cout << "-------------------------------------" << std::endl;
				}
			}
			#endif
		}
	}
	catch (std::exception &e)
	{
		std::cerr << e.what();
		return (6);
	}
	return (0);
}

int main(int argc, char **argv) {
	std::filesystem::path logFilePath(PATH_LOGFILE);
	if (std::filesystem::exists(logFilePath))
		std::remove(PATH_LOGFILE);

	std::shared_ptr<Log> log = std::make_shared<Log>();
	char *buffer = NULL;
	int		parse_code = 0;
	std::string		config_file_name;
	std::shared_ptr<ServerConnection> ptr_ServerConnection = std::make_shared<ServerConnection>(log);
	ClientConnection clientConnection(ptr_ServerConnection, log);
	std::list<ServerStruct> server_structs;
	Parser parser;

	if (argc == 1)
	{
		config_file_name = STD_CONFIG;
	}
	else if (argc != 2) {
		std::cerr << "Config file is missing." << std::endl;
		error_exit(1);
		return (1);
	}
	else
	{
		config_file_name = *(argv + 1);
	}
	std::cout << "\033[32mWebserv started ...\033[0m" << std::endl;
	log->logAdd("Webserv started.");

	initSignals();
	parse_code = parse(&parser, &server_structs, &buffer, config_file_name);
	if (parse_code)
	{
		if (buffer)
			delete[] buffer;
		error_exit(parse_code);
		return (1);

	}
	for (auto &server : server_structs)
		ptr_ServerConnection->setUpServerConnection(server);
	clientConnection.setupClientConnection(&server_structs);
	delete[] buffer;

	std::cout << "\033[33mWebserv closed.\033[0m" << std::endl;
	return 0;
}
