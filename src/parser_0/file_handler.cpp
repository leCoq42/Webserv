#include "inc/parser.hpp"

int	load_file_to_buff(char *file_name, char **buffer, int *file_len)
{
	std::fstream	inputFile;

	inputFile.open(file_name, std::ios::in);
	if (!inputFile)
	{
		std::cout << "file not found" << std::endl;
		return (1);
	}
	inputFile.seekg(0, std::ios::end);
	*file_len = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);
	*buffer = new char[*file_len + 1];
	if (!buffer)
		return (2);
	inputFile.read(*buffer, *file_len);
	*(*buffer + *file_len) = 0;
	inputFile.close();
	return (0);
}
