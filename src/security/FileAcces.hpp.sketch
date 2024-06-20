#include "../../inc/Webserv.hpp"

class	FileAcces
{
	private:
	ServerStruct				&config;
	std::vector<std::string>	uploadedFiles;
	public:
	FileAcces(ServerStruct &config);
	~FileAcces();
	bool	isFilePermissioned(std::string file);
	void	addFile(std::string file);
};
