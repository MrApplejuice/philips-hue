/*  This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
    Author: Paul Konstantin Gerke (email: paulkgerke(a)craftware.nl)  
*/


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
