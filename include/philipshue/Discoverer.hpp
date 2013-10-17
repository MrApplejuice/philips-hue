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

#pragma once

#include "Config.hpp"

#include <string>
#include <vector>
#include <stdexcept>

#include <curl/curl.h>

#include <glib-2.0/glib.h>
#include <libgssdp/gssdp.h>

#include "DeviceAddress.hpp"

namespace philips {
  namespace hue {
    class DiscoveredDeviceAddress : public virtual DeviceAddress {
      public:
        typedef Pointer<DiscoveredDeviceAddress>::type Ptr;
      private:
        std::string descriptorUrl;
        
        bool connected;
        std::string url;
      public:
        virtual std::string getURL();
        
        virtual std::string getDescriptorURL() const;
        
        DiscoveredDeviceAddress& operator=(const DiscoveredDeviceAddress& other);
        
        DiscoveredDeviceAddress();
        DiscoveredDeviceAddress(const std::string& descriptorUrl);
        DiscoveredDeviceAddress(const DiscoveredDeviceAddress& other);
    };
    
    class Discoverer {
      public:
        typedef DiscoveredDeviceAddress Address;
        typedef std::vector<Address::Ptr> AddressPtrVector;
        typedef std::vector<Address> AddressVector;
        
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
        
        AddressPtrVector addresses, removedAddresses;

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
        
        void addDiscoveredAddress(const std::string& descriptorUrl);
      public:
        virtual AddressVector getAddresses();
        
        virtual TimeoutDelay getRescanInterval();
        virtual void setRescanInterval(TimeoutDelay interval);
      
        Discoverer();
        Discoverer(const std::string& interface);
        virtual ~Discoverer();
    };
  }
}
