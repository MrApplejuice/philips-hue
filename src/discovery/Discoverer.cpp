#include <philipshue/Discoverer.hpp>

namespace philips {
  namespace hue {
    std::string DiscoveredDeviceAddress :: getURL() {
      return url;
    }

    void Discoverer :: init(const std::string* address) {
      GError* error;
      client = gssdp_client_new(NULL, address == NULL ? NULL : address->c_str(), &error);
      if (client == NULL) {
        throw Exception(std::string("Could not create client: ") + error->message);
      }
    }
    
    Discoverer::AddressPtrVector Discoverer :: getAddresses() {
    }
  
    Discoverer :: Discoverer() : client(NULL) {
    }
    
    Discoverer :: ~Discoverer() {
      if (client != NULL) {
        g_object_unref(client);
        client = NULL;
      }
    }
  }
}
