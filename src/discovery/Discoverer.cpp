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

#include <philipshue/Discoverer.hpp>

#define PHILIPS_HUE_UUID "uuid:2f402f80-da50-11e1-9b23-0017880a83d4"

#include <iostream>

namespace philips {
  namespace hue {
    std::string DiscoveredDeviceAddress :: getURL() {
      if (connected) {
        return url;
      }
      return std::string();
    }

    std::string DiscoveredDeviceAddress :: getDescriptorURL() const {
      return descriptorUrl;
    }
    
    DiscoveredDeviceAddress& DiscoveredDeviceAddress :: operator=(const DiscoveredDeviceAddress& other) {
      descriptorUrl = other.descriptorUrl;

      connected = other.connected;
      url = other.url;
      
      return *this;
    }
    
    DiscoveredDeviceAddress :: DiscoveredDeviceAddress() : connected(false) {
      descriptorUrl = "";
    }
    
    DiscoveredDeviceAddress :: DiscoveredDeviceAddress(const std::string& descriptorUrl) : connected(false) {
      curl_global_init(CURL_GLOBAL_DEFAULT);
      this->descriptorUrl = descriptorUrl;
    }
    
    DiscoveredDeviceAddress :: DiscoveredDeviceAddress(const DiscoveredDeviceAddress& other) {
      this->operator=(other);
    }


    void Discoverer :: init(const std::string* address) {
      GError* error = NULL;
      
      // glib context and mainloop initialization
      context = g_main_context_new();
      if (context == NULL) {
        throw Exception(std::string("Could not create glib context"));
      }
      g_main_context_push_thread_default(context);
      
      mainLoop = g_main_loop_new(context, false);
      if (mainLoop == NULL) {
        throw Exception(std::string("Could not create glib main loop"));
      }
      
      // ssdp discovery initialization
      client = gssdp_client_new(NULL, address == NULL ? NULL : address->c_str(), &error);
      if (client == NULL) {
        throw Exception(std::string("Could not create client: ") + error->message);
      }
      
      browser = gssdp_resource_browser_new(client, PHILIPS_HUE_UUID);
      if (browser == NULL) {
        throw Exception(std::string("Could not create ssdp browser"));
      }
      gssdp_resource_browser_set_active(browser, true);
      
      // Connect signals
      g_signal_connect(browser, "resource-available", reinterpret_cast<GCallback>(&handleResourceAvailableStatic), this);
      g_signal_connect(browser, "resource-unavailable", reinterpret_cast<GCallback>(&handleResourceUnavailableStatic), this);

      // Initialize rescan timeouts
      rescanInterval = 0; // deactivated
      currentRescanInterval = rescanInterval;

      // Start mainloop thread      
      g_rec_mutex_init(&mainLoopMutex); // Seems this cannot fail
      
      mainLoopThread = g_thread_new(NULL, &mainLoopThreadFuncStatic, this);
      if (mainLoopThread == NULL) {
        throw Exception(std::string("Could not create thread"));
      }
      
      g_main_context_pop_thread_default(context);
    }
    
    void Discoverer :: registerNewRescanTimer() {
      GSource* source = g_timeout_source_new(currentRescanInterval);
      g_source_set_callback(source, &handleRescanTimeoutStatic, this, NULL);
      g_source_attach(source, context);
      g_source_unref(source);
    }
    
    gpointer Discoverer :: mainLoopThreadFuncStatic(gpointer userdata) {
      return GINT_TO_POINTER(static_cast<Discoverer*>(userdata)->mainLoopThreadFunc());
    }
    
    gint Discoverer :: mainLoopThreadFunc() {
      g_main_loop_run(mainLoop);
      return 0;
    }
    
    void Discoverer :: handleResourceAvailableStatic(GSSDPResourceBrowser *resource_browser, gchar *usn, gpointer locations, gpointer userdata) {
      static_cast<Discoverer*>(userdata)->handleResourceAvailable(usn, locations);
    }
    
    void Discoverer :: handleResourceAvailable(gchar *usn, gpointer locations) {
      GList* list = static_cast<GList*>(locations);
      
      while (list != NULL) {
        std::string descriptorUrl = std::string(static_cast<char*>(list->data));
        addDiscoveredAddress(descriptorUrl);
        list = list->next;
      }
    }
    
    void Discoverer :: handleResourceUnavailableStatic(GSSDPResourceBrowser *resource_browser, gchar *usn, gpointer userdata) {
      static_cast<Discoverer*>(userdata)->handleResourceUnavailable(usn);
    }
    
    void Discoverer :: handleResourceUnavailable(gchar *usn) {
      // "Un-discovery" is done in handleRescaneTimeout by throwing away all "removedAddresses"
    }

    gboolean Discoverer :: handleRescanTimeoutStatic(gpointer userdata) {
      return static_cast<Discoverer*>(userdata)->handleRescanTimeout();
    }
    
    bool Discoverer :: handleRescanTimeout() {
      bool result = true;
      
      // Adapt update interval if it changed
      g_rec_mutex_lock(&mainLoopMutex);
      if (rescanInterval != currentRescanInterval) {
        result = false;
        currentRescanInterval = rescanInterval;
        if (currentRescanInterval > 0) {
          registerNewRescanTimer();
        }
      }
      
      // Adapt list of detected/undetected devices
      removedAddresses = addresses;
      addresses.clear();
      
      g_rec_mutex_unlock(&mainLoopMutex);

      // Start rescan
      gssdp_resource_browser_set_active(browser, false);
      gssdp_resource_browser_set_active(browser, true);
      gssdp_resource_browser_rescan(browser); // this alone does not do the trick :-(
      return result;
    }
    
    void Discoverer :: addDiscoveredAddress(const std::string& descriptorUrl) {
      g_rec_mutex_lock(&mainLoopMutex);
      bool recovered = false;
      for (AddressPtrVector::iterator it = removedAddresses.begin(); it != removedAddresses.end(); it++) {
        if (descriptorUrl == (*it)->getDescriptorURL()) {
          addresses.push_back(*it);
          removedAddresses.erase(it);
          recovered = true;
          break;
        }
      }
      
      if (!recovered) {
        addresses.push_back(DiscoveredDeviceAddress::Ptr(new DiscoveredDeviceAddress(descriptorUrl)));
      }
      g_rec_mutex_unlock(&mainLoopMutex);
    }
    
    Discoverer::AddressVector Discoverer :: getAddresses() {
      g_rec_mutex_lock(&mainLoopMutex);
      AddressVector result;
      result.reserve(addresses.size());
      for (AddressPtrVector::iterator it = addresses.begin(); it != addresses.end(); it++) {
        result.push_back(**it);
      }
      for (AddressPtrVector::iterator it = removedAddresses.begin(); it != removedAddresses.end(); it++) {
        result.push_back(**it);
      }
      g_rec_mutex_unlock(&mainLoopMutex);
      return result;
    }
  
    Discoverer::TimeoutDelay Discoverer :: getRescanInterval() {
      g_rec_mutex_lock(&mainLoopMutex);
      int result = rescanInterval;
      g_rec_mutex_unlock(&mainLoopMutex);
      return result;
    }
    
    void Discoverer :: setRescanInterval(Discoverer::TimeoutDelay interval) {
      g_rec_mutex_lock(&mainLoopMutex);
      rescanInterval = interval;
      if ((rescanInterval > 0) && (currentRescanInterval <= 0)) { // were timeouts deactivated all together?!
        currentRescanInterval = interval;
        registerNewRescanTimer();
      }
      g_rec_mutex_unlock(&mainLoopMutex);
    }
  
    Discoverer :: Discoverer() : mainLoopThread(NULL), mainLoopMutex(), context(NULL), mainLoop(NULL), client(NULL), browser(NULL), addresses(), removedAddresses() {
      init(NULL);
    }
    
    Discoverer :: Discoverer(const std::string& interface) : mainLoopThread(NULL), mainLoopMutex(), context(NULL), mainLoop(NULL), client(NULL), browser(NULL), addresses(), removedAddresses() {
      init(&interface);
    }
    
    Discoverer :: ~Discoverer() {
      if (mainLoopThread != NULL) {
        while (!g_main_loop_is_running(mainLoop)) {g_thread_yield();} // Forced wait until main loop is running
        g_main_loop_quit(mainLoop);
        g_thread_join(mainLoopThread);
        g_thread_unref(mainLoopThread);
        mainLoopThread = NULL;
      }
      
      g_rec_mutex_clear(&mainLoopMutex);
      
      g_clear_object(&browser);
      g_clear_object(&client);

      if (mainLoop != NULL) {
        g_main_loop_unref(mainLoop);
        mainLoop = NULL;
      }
      
      if (context != NULL) {
        g_main_context_unref(context);
        context = NULL;
      }
    }
  }
}

#include "CurlInitializer.icpp"
