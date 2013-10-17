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
        
        typedef unsigned int TimeoutDelay;
        
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
        
        GSource* rescanTimeout;
        
        TimeoutDelay rescanInterval, currentRescanInterval;

        void init(const std::string* address);
        void registerNewRescanTimer();

        static gpointer mainLoopThreadFuncStatic(gpointer userdata);
        gint mainLoopThreadFunc();
        
        static void handleResourceAvailableStatic(GSSDPResourceBrowser *resource_browser, gchar *usn, gpointer locations, gpointer userdata);
        void handleResourceAvailable(gchar *usn, gpointer locations);

        static void handleResourceUnavailableStatic(GSSDPResourceBrowser *resource_browser, gchar *usn, gpointer userdata);
        void handleResourceUnavailable(gchar *usn);
        
        static gboolean handleRescanTimeoutStatic(gpointer userdata);
        bool handleRescanTimeout();
      public:
        virtual AddressPtrVector getAddresses();
        
        virtual TimeoutDelay getRescanInterval();
        virtual void setRescanInterval(TimeoutDelay interval);
      
        Discoverer();
        Discoverer(const std::string& interface);
        virtual ~Discoverer();
    };
  }
}
