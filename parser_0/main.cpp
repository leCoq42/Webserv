#include "main.hpp"

void	error_exit(int error_code)
{
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

int	main(int argc, char **argv)
{
	Parser					parser("#", "\n", "{", "}", " 	\n", "'", " 	\n", ";");
	std::list<ServerStruct>	server_structs;
	int						file_len;
	char					*buffer;

	if (argc != 2)
		error_exit(1);
	if (load_file_to_buff(*(argv + 1), &buffer, &file_len))
		error_exit(2);
	if (!parser.parse_content_to_struct(buffer, file_len))
		error_exit(3);
	std::cout << parser.PS.n_servers << std::endl;
	if (!load_in_servers(&parser.PS, server_structs))
		error_exit(4);
	if (!server_structs.empty())
		for (ServerStruct server : server_structs)
		{
			std::cout << "-------------------------------------" << std::endl;
			server.show_self();
			std::cout << "-------------------------------------" << std::endl;
		}
	delete buffer;
}
