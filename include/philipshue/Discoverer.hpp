#pragma once

#include "Config.hpp"

#include <string>
#include <vector>
#include <stdexcept>

#include <glib-2.0/glib.h>
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
        GThread* mainLoopThread;
        GRecMutex mainLoopMutex;
      
        GMainContext* context;
        GMainLoop* mainLoop;
      
        GSSDPClient* client;
        GSSDPResourceBrowser* browser;

        void init(const std::string* address);

        static gpointer mainLoopThreadFuncStatic(gpointer userdata);
        gint mainLoopThreadFunc();
        
        static void handleResourceAvailableStatic(GSSDPResourceBrowser *resource_browser, gchar *usn, gpointer locations, gpointer userdata);
        void handleResourceAvailable(gchar *usn, gpointer locations);
      public:
        virtual AddressPtrVector getAddresses();
      
        Discoverer();
        Discoverer(const std::string& interface);
        virtual ~Discoverer();
    };
  }
}
