#include "ClientConnection.hpp"
#include "log.hpp"
#include "Parser.hpp"
#include "ServerConnection.hpp"
#include "ServerStruct.hpp"
#include "fileHandler.hpp"
#include "signals.hpp"
#include <iostream>
#include <memory>

void error_exit(int error_code) {
	if (error_code == 1)
		std::cerr << "no file given" << std::endl;
	else if (error_code == 2)
		std::cerr << "file loading error" << std::endl;
	else if (error_code == 3)
		std::cerr << "invalid config file" << std::endl;
	else if (error_code == 4)
		std::cerr << "loading server struct went wrong" << std::endl;
	exit(1);
}

void parse(Parser *parser, std::list<ServerStruct> *server_structs,
           char **buffer, char **argv) {
	int file_len;

	if (load_file_to_buff(*(argv + 1), buffer, &file_len))
		error_exit(2);
	if (!parser->parse_content_to_struct(*buffer, file_len))
		error_exit(3);
	if (!load_in_servers(&parser->PS, *server_structs))
		error_exit(4);
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

int main(int argc, char **argv) {
	std::filesystem::path logFilePath(PATH_LOGFILE);
	if (std::filesystem::exists(logFilePath))
		std::remove(PATH_LOGFILE);
	Log _log;
	char *buffer;
	std::shared_ptr<ServerConnection> SS = std::make_shared<ServerConnection>();
	ClientConnection CC(SS);
	std::list<ServerStruct> server_structs;
	Parser parser("#", "\n", "{", "}", " 	\n", "'", " 	\n", ";");

	if (argc != 2) {
		std::cerr << "Config file is missing." << std::endl;
		error_exit(1);
	}
	std::cout << "\033[32mWebserv started ...\033[0m" << std::endl;
	_log.logAdd("Webserv started.");


	initSignals();
	parse(&parser, &server_structs, &buffer, argv);


	for (auto &server : server_structs)
		SS->setUpServerConnection(server);
	CC.setupClientConnection(&server_structs);
	delete[] buffer;

	std::cout << "\033[33mWebserv closed.\033[0m" << std::endl;
	return 0;
}
