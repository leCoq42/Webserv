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
    std::cout << "no file given" << std::endl;
  else if (error_code == 2)
    std::cout << "file loading error" << std::endl;
  else if (error_code == 3)
    std::cout << "parsing went wrong" << std::endl;
  else if (error_code == 4)
    std::cout << "loading server struct went wrong" << std::endl;
  exit(1);
}

void parse(Parser *parser, std::list<ServerStruct> *server_structs,
           char **buffer, char **argv) {
  int file_len;

  if (load_file_to_buff(*(argv + 1), buffer, &file_len))
    error_exit(2);
  if (!parser->parse_content_to_struct(*buffer, file_len))
    error_exit(3);
  std::cout << parser->PS.get_nServers() << std::endl;
  if (!load_in_servers(&parser->PS, *server_structs))
    error_exit(4);
  // DEBUG
  if (!server_structs->empty()) {
    for (ServerStruct server : *server_structs) {
      std::cout << "-------------------------------------" << std::endl;
      server.show_self();
      std::cout << "-------------------------------------" << std::endl;
    }
  }
}

int main(int argc, char **argv) {
	char *buffer;
	std::shared_ptr<ServerConnection> SS = std::make_shared<ServerConnection>();
	ClientConnection CC(SS);
	std::list<ServerStruct> server_structs;
	Parser parser("#", "\n", "{", "}", " 	\n", "'", " 	\n", ";");

	if (argc != 2)
		error_exit(1);
	initSignals();
	parse(&parser, &server_structs, &buffer, argv);
	for (auto &server : server_structs)
		SS->setUpServerConnection(server);
	CC.setupClientConnection();
	delete[] buffer;
	return 0;
}
