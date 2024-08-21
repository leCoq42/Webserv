#include <ctime>
#include <iostream>

int main() {
  std::cout << "Content-Type: " << "text/html\r\n\r\n";
  std::cout << "<!DOCTYPE html><html lang=\"en\"><head><title>Datetime</title></head><body>"
		<< std::endl;

  time_t currentTime;
  time(&currentTime);
  std::cout << asctime(localtime(&currentTime));

  std::cout << "</body></html>";
  return 0;
}
