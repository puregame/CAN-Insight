#include "datatypes.h"
#include "config.h"
#include <ArduinoJson.h>
#include <WiFi101.h>

#ifndef wifi_manager_
#define wifi_manager_
class Wifi_Network{
    public:
        char ssid[33];
        char password[64];
};

class Wifi_Manager{
    public:
        void check_wifi_netowrks(Wifi_Network* networks);
        bool is_connected();
        int get_status();
        Wifi_Network possible_networks[MAX_SAVED_NETWORK_COUNT];
        WiFiClient wifi;
        void print_wifi_status();

        
};
#endif
