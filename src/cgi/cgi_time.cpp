#include <ctime>
#include <iostream>

int main() {
  time_t currentTime; // time_t defined in <ctime>
  time(&currentTime);
  // asctime and localtime defined in <ctime>
  std::cout << asctime(localtime(&currentTime));
  return 0;
}
