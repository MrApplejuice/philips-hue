#include <philipshue/Discoverer.hpp>

#define PHILIPS_HUE_UUID "uuid:2f402f80-da50-11e1-9b23-0017880a83d4"

#include <iostream>

namespace philips {
  namespace hue {
    std::string DiscoveredDeviceAddress :: getURL() {
      return url;
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
      std::cout << "Discovered " << usn << std::endl;
      GList* list = static_cast<GList*>(locations);
      while (list != NULL) {
        std::cout << "  " << static_cast<char*>(list->data) << std::endl;
        list = list->next;
      }
    }
    
    void Discoverer :: handleResourceUnavailableStatic(GSSDPResourceBrowser *resource_browser, gchar *usn, gpointer userdata) {
      static_cast<Discoverer*>(userdata)->handleResourceUnavailable(usn);
    }
    
    void Discoverer :: handleResourceUnavailable(gchar *usn) {
      std::cout << "Lost " << usn << std::endl;
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
      g_rec_mutex_unlock(&mainLoopMutex);

      // Start rescan
      gssdp_resource_browser_set_active(browser, false);
      gssdp_resource_browser_set_active(browser, true);
      gssdp_resource_browser_rescan(browser); // this alone does not do the trick :-(
      return result;
    }
    
    Discoverer::AddressPtrVector Discoverer :: getAddresses() {
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
  
    Discoverer :: Discoverer() : mainLoopThread(NULL), mainLoopMutex(), context(NULL), mainLoop(NULL), client(NULL), browser(NULL) {
      init(NULL);
    }
    
    Discoverer :: Discoverer(const std::string& interface) : mainLoopThread(NULL), mainLoopMutex(), context(NULL), mainLoop(NULL), client(NULL), browser(NULL) {
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
