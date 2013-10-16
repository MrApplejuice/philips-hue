#pragma once

#include <memory>

namespace philips {
  namespace hue {
    // Which pointer type to use to hold references to objects
    template<class T>
    struct Pointer {
      typedef std::auto_ptr<T> type;
    };
  }
}
