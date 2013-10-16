#include <philipshue/PhilipsHue.hpp>

#include <string>
#include <iostream>

int main(int argc, char** argv) {
  philips::hue::Discoverer discoverer;
  
  std::string line;
  std::getline(std::cin, line);
  
  return 0;
}
