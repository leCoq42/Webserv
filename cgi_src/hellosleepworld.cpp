#include <ctime>
#include <iostream>
#include <unistd.h>

int main() {
  sleep(60);
  std::cout << "Content-Type: " << "text/html\r\n\r\n";
  std::cout << "<!DOCTYPE html><html "
               "lang=\"en\"><head><title>Datetime</title></head><body>"
            << std::endl;

  std::cout << "Hello world!" << std::endl;
  std::cout << "Hello world!" << std::endl;

  std::cout << "</body></html>";
  return 0;
}
