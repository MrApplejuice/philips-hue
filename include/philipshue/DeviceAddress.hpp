#pragma once

#include "Config.hpp"

#include <string>

namespace philips {
  namespace hue {
    class DeviceAddress {
      public:
        typedef Pointer<DeviceAddress>::type Ptr;
        
        virtual std::string getURL() = 0;
    };
  }
}
