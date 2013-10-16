#pragma once

#include "Config.hpp"

#include <string>
#include <vector>
#include <stdexcept>

#include <libgssdp/gssdp.h>

#include "DeviceAddress.hpp"

namespace philips {
  namespace hue {
    class DiscoveredDeviceAddress : public virtual DeviceAddress {
      public:
        typedef Pointer<DiscoveredDeviceAddress>::type Ptr;
      private:
        std::string url;
      public:
        virtual std::string getURL();
    };
    
    class Discoverer {
      public:
        typedef DiscoveredDeviceAddress Address;
        typedef std::vector<Address::Ptr> AddressPtrVector;
        
        class Exception : public std::runtime_error {
          public:
            Exception(const std::string& error) throw() : std::runtime_error(error) {}
        };
      private:
        GSSDPClient* client;
        GSSDPResourceBrowser* browser;
        
        void init(const std::string* address);
      public:
        virtual AddressPtrVector getAddresses();
      
        Discoverer();
        virtual ~Discoverer();
    };
  }
}
