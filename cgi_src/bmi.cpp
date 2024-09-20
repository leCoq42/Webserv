#include <iostream>

float bmi(float weight, float height) { return weight / (height * height); }

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <weight> <height>" << std::endl;
    return 1;
  }

  float weight = std::stof(argv[1]);
  float height = std::stof(argv[2]);
  float result = bmi(weight, height);
  std::cout << "BMI = " << result << std::endl;
  return 0;
}
