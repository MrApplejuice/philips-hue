#include <philipshue/PhilipsHue.hpp>

#include <string>
#include <iostream>

#include <boost/lexical_cast.hpp>

int main(int argc, char** argv) {
  philips::hue::Discoverer discoverer;
  
  std::string line;
  do {
    std::getline(std::cin, line);
    try {
      philips::hue::Discoverer::TimeoutDelay i = boost::lexical_cast<philips::hue::Discoverer::TimeoutDelay>(line);
      discoverer.setRescanInterval(i);
    }
    catch (boost::bad_lexical_cast) {
    }
  } while (line != "exit");
  
  return 0;
}
