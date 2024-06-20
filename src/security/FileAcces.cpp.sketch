#include "FileAcces.hpp"

FileAcces::FileAcces(ServerStruct &config): config(config)
{

}

FileAcces::~FileAcces()
{

}

bool	FileAcces::isFilePermissioned(std::string file)
{
	int i;

	//check if in root
	i = uploadedFiles.size();
	while (i--)
		if (!uploadedFiles[i].compare(file))
			return true;
	//look through locations in server
	return false;
}

void	FileAcces::addFile(std::string file) //security issue if file is already there
{
	uploadedFiles.push_back(file);
}
