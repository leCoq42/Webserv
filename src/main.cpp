#include "webserv.hpp"
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
  std::cout << parser->PS.n_servers << std::endl;
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
  auto SS = std::make_shared<ServerConnection>();
  ClientConnection CC(SS);
  Parser parser("#", "\n", "{", "}", " 	\n", "'", " 	\n", ";");
  std::list<ServerStruct> server_structs;
  char *buffer;

  if (argc != 2)
    error_exit(1);
  parse(&parser, &server_structs, &buffer, argv);
  std::cout << std::endl << std::endl;
  for (const auto &server : server_structs)
    SS->setUpServerConnection(server);
  CC.setUpClientConnection();
  delete buffer;

  // Log logger;
  // for (int i = 0; i < 10; i++)
  // 	logger.logAccess("127.0.0.1", "GET /index.html HTTP/1.1", 200, 612, "-",
  // "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like
  // Gecko) Chrome/91.0.4472.124 Safari/537.36" );
}
