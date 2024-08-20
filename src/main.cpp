#include "ClientConnection.hpp"
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
}

int parse(Parser *parser, std::list<ServerStruct> *server_structs,
           char **buffer, char **argv) {
	int file_len;

	if (load_file_to_buff(*(argv + 1), buffer, &file_len))
		return (2);
	else if (!parser->parse_content_to_struct(*buffer, file_len))
		return (3);
	else if (!load_in_servers(&parser->PS, *server_structs))
		return (4);
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
	return (0);
}

int main(int argc, char **argv) {
	char	*buffer = NULL;
	int		parse_code = 0;

	std::shared_ptr<ServerConnection> SS = std::make_shared<ServerConnection>();
	ClientConnection CC(SS);
	std::list<ServerStruct> server_structs;
	Parser parser("#", "\n", "{", "}", " 	\n", "'", " 	\n", ";");

	if (argc != 2)
		error_exit(1);

	initSignals();
	parse_code = parse(&parser, &server_structs, &buffer, argv);
	if (parse_code)
	{
		if (buffer)
			delete[] buffer;
		error_exit(parse_code);
		return (1);

	}
	for (auto &server : server_structs)
		SS->setUpServerConnection(server);
	CC.setupClientConnection(&server_structs);
	delete[] buffer;
	return 0;
}
