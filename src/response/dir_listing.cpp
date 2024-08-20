#include "dir_listing.hpp"

std::string &add_dir_to_listing(std::string &return_html, char *directory, const std::string &path, const std::string &referer)
{
	(void)referer; // tja
	// std::size_t	ith;
	// // std::size_t	ref_ith;

	// ith = 0;
	// // ref_ith = 0;
	// if (referer.size() > 8)
	// 	ith = referer.find(8, '/'); // SLORDIG, hoezo staat dit in de main?
	return_html += "<button onclick = \"window.location.href='";
	if (return_html.back() != '/')
		return_html += '/';
	return_html += path;
	if (return_html.back() != '/' && return_html.back() != '.') return_html += '/';//if (return_html.back() != '/') return_html += '/';
	return_html += directory;
	return_html += "';\">";
	return_html += directory;
	return_html += "</button>";
	return (return_html);
}

std::string list_dir(std::filesystem::path &path, const std::string &uri, const std::string &referer, int &status_code)
{
	DIR			*dir;
	std::string	return_html;
	std::string	relative_path;

	relative_path = path;
	// std::cout << "PATH:" << relative_path << std::endl;
	return_html = "<!DOCTYPE html><html><head><title>";
	return_html += uri;
	return_html += "</title></head><body>";
	return_html += "<h1>Directory view of: " + relative_path + "</h1>";
	return_html += "<p>Referer: " + referer + "</p>";
	dir = opendir(relative_path.c_str());
	if (dir)
	{
		struct	dirent *directory;
	
		directory = readdir(dir);
		// std::cout << directory->d_name << std::endl;
		while (directory)
		{
			// std::cout << directory->d_name << std::endl;
			directory = readdir(dir);
			if (directory)
				return_html = add_dir_to_listing(return_html, directory->d_name, uri, referer);
		}
		closedir(dir);
	}
	else
	{
		status_code = 404;
		return_html += "<h2>Directory not found.</h2>";
	}
	return_html += "</body></html>";
	// std::cout << "PATH:" << relative_path << std::endl;
	return (return_html);
}

std::string	redirect(std::string redirect_string)
{
	std::string	response_str;

	response_str = "HTTP/1.1 301 Moved Permanently\nLocation: ";
	response_str += redirect_string;
	return response_str;
}

std::string	standard_error(int error_code, std::string error_description)
{
	std::string	response_str;

	response_str = "<!DOCTYPE html><html lang=\"en\"><head><title>";
	response_str += std::to_string(error_code);
	response_str += error_description;
	response_str += "</title></head><body><div class=\"error-container\"><h1> ";
	response_str += std::to_string(error_code);
	response_str += " </h1><p>";
	response_str += error_description;
	response_str += "</p><a href=\"/\">Go Back to Home</a></div></body></html>";
	return (response_str);
}
