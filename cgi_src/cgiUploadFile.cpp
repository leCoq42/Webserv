#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

const int BUFFER_SIZE = 1024;

std::string urlDecode(const std::string &input) {
  std::string result;
  for (size_t i = 0; i < input.length(); ++i) {
    if (input[i] == '%') {
      if (i + 2 < input.length()) {
        int value;
        std::sscanf(input.substr(i + 1, 2).c_str(), "%x", &value);
        result += static_cast<char>(value);
        i += 2;
      }
    } else if (input[i] == '+') {
      result += ' ';
    } else {
      result += input[i];
    }
  }
  return result;
}

void parseMultipartFormData(const std::string &boundary) {
  std::string line;
  std::string filename;
  std::ofstream outFile;
  bool isFile = false;

  while (std::getline(std::cin, line)) {
    if (line.find("Content-Disposition:") != std::string::npos) {
      size_t filenamePos = line.find("filename=\"");
      if (filenamePos != std::string::npos) {
        size_t filenameEnd = line.find("\"", filenamePos + 10);
        filename =
            line.substr(filenamePos + 10, filenameEnd - filenamePos - 10);
        filename = urlDecode(filename);
        outFile.open(filename, std::ios::binary);
        isFile = true;
      }
    } else if (line == "\r") {
      if (isFile) {
        char buffer[BUFFER_SIZE];
        while (std::cin.read(buffer, BUFFER_SIZE)) {
          std::string chunk(buffer, std::cin.gcount());
          size_t boundaryPos = chunk.find(boundary);
          if (boundaryPos != std::string::npos) {
            outFile.write(chunk.c_str(), boundaryPos - 2);
            break;
          }
          outFile.write(chunk.c_str(), chunk.length());
        }
        outFile.close();
        isFile = false;
        break;
      }
    }
  }
}

int main() {
  std::cout << "Content-type: text/html\r\n\r\n";
  std::cout << "<html><body>";

  std::string contentType = std::getenv("CONTENT_TYPE");
  std::string boundary = std::getenv("BOUNDARY");

  if (!contentType.empty() &&
      std::strstr(contentType.c_str(), "multipart/form-data") != nullptr) {
    // std::string boundaryStart = std::strstr(contentType.c_str(),
    // "boundary=");
    if (!boundary.empty()) {
      // std::string boundary = "--";
      // boundary += (boundaryStart.c_str() + 9);
      std::cout << "Boundary= " << boundary << std::endl;
      parseMultipartFormData(boundary);
      std::cout << "<h2>File uploaded successfully!</h2>";
    } else {
      std::cout << "<h2>Error: No boundary found in multipart/form-data</h2>";
    }
  } else {
    std::cout << "<h2>Error: Invalid content type. Expected "
                 "multipart/form-data.</h2>";
  }

  std::cout << "</body></html>";
  return 0;
}
